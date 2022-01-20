#include "game.h"
#include "body.h"
#include "graphics.h"
#include "key_listener.h"
#include "physics.h"
#include "sdl_wrapper.h"
#include "vector.h"
#include <SDL2/SDL_timer.h>
#include <assert.h>
#include <stdio.h>

/*** GLOBALS ***/
int _game_count = 0;

/*** TYPES ***/
struct game {
    list_t **groups;
    size_t groups_count;
    physics_t *physics;
    graphics_t *graphics;
    tick_func_t tick_func;
    void *aux;
    free_func_t aux_freer;
    list_t *timers;
    key_listener_t *key_listener;
};

typedef struct _game_timer {
    SDL_TimerID id;
    SDL_TimerCallback callback;
    void *aux;
    Uint32 interval;
    bool removed; // Whether marked for removal.
    bool called;  // Whether has emerged from the queue yet, can be freed.
} _game_timer_t;

typedef enum _game_event_code { _GAME_EVENT_CODE_TIMER } _game_event_code_t;

/*** PRIVATE PROTOTYPES ***/
void _game_collect_garbage(game_t *game);
Uint32 _game_callback(Uint32 interval, _game_timer_t *timer);

/**
 * Remove a timer and its tracker.
 */
void _game_timer_free(_game_timer_t *timer);

/*** DEFINITIONS ***/

game_t *game_init(vector_t dims,
                  size_t groups_count,
                  tick_func_t tick_func,
                  void *aux,
                  free_func_t aux_freer) {
    if (_game_count > 0) {
        fprintf(stderr, "Invalid state: you cannot have multiple games.");
        exit(1);
    }
    game_t *game = malloc(sizeof(game_t));
    assert(game != NULL);
    sdl_init(VEC_ZERO, dims);
    game->groups_count = groups_count;
    game->groups = calloc(groups_count, sizeof(list_t *));
    assert(game->groups != NULL);
    for (size_t i = 0; i < game->groups_count; i++) {
        game->groups[i] = list_init(1, (free_func_t)body_free);
    }
    game->physics = physics_init();
    game->graphics = graphics_init(dims);
    game->tick_func = tick_func;
    game->aux = aux;
    game->aux_freer = aux_freer;
    game->timers = list_init(1, (free_func_t)_game_timer_free);
    game->key_listener = key_listener_init();
    _game_count++;
    return game;
}

void game_free(game_t *game) {
    graphics_free(game->graphics);
    physics_free(game->physics);
    for (size_t i = 0; i < game->groups_count; i++) {
        list_free(game_get_group(game, i));
    }
    free(game->groups);
    if (game->aux_freer != NULL) {
        game->aux_freer(game->aux);
    }
    list_free(game->timers);
    key_listener_free(game->key_listener);
    sdl_free_all();
    free(game);
    _game_count--;
}

void _game_audit_gc(game_t *game, char *loc) {
    for (size_t i = 0; i < game->groups_count; i++) {
        list_t *bodies = game_get_group(game, i);
        for (size_t j = 0; j < list_size(bodies); j++) {
            if (body_is_removed(list_get(bodies, j))) {
                fprintf(stderr,
                        "Invalid state: body idx %zu with group idx %zu marked "
                        "for removal at %s.\n",
                        i,
                        j,
                        loc);
                assert(false && "See error on stderr.");
            }
        }
    }
}

bool game_tick(game_t *game, double dt) {
    // Nothing should be marked for removal at the beginning of the tick, and
    // any frees should be deferred to _game_collect_garbage at the very end
    // of the tick, including any removal by tick_func.
    _game_audit_gc(game, "game_tick start");

    bool done = false;

    // Handle sdl events.
    SDL_Event *event = malloc(sizeof(SDL_Event));
    assert(event != NULL);
    while (SDL_PollEvent(event)) {
        switch (event->type) {
        case SDL_QUIT:
            done = true;
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            if (!event->key.repeat) {
                key_listener_listen(game->key_listener, event->key);
            }
            break;
        case SDL_USEREVENT:
            if (event->user.code == _GAME_EVENT_CODE_TIMER) {
                _game_timer_t *timer = event->user.data1;
                if (!timer->removed && !timer->called) {
                    Uint32 interval
                        = timer->callback(timer->interval, timer->aux);
                    if (interval > 0) {
                        game_add_timer(game,
                                       interval,
                                       timer->callback,
                                       timer->aux);
                    }
                }
                timer->removed = true;
                timer->called = true;
            }
            break;
        }
    }
    free(event);

    // Tick components.
    physics_tick(game->physics, dt);
    graphics_render(game->graphics);

    // Client's custom tick.
    done = done || game->tick_func(game);

    // Garbage collection, actual freeing happens here and only here.
    _game_collect_garbage(game);

    return done;
}

void _game_collect_garbage(game_t *game) {
    for (int i = list_size(game->timers) - 1; i >= 0; i--) {
        _game_timer_t *timer = list_get(game->timers, i);
        if (timer->removed && timer->called) {
            _game_timer_free(list_remove(game->timers, i));
        }
    }
    for (size_t i = 0; i < game->groups_count; i++) {
        list_t *bodies = game_get_group(game, i);
        for (int j = list_size(bodies) - 1; j >= 0; j--) {
            body_t *body = list_get(bodies, j);
            if (body_is_removed(body)) {
                body_free(list_remove(bodies, j));
            }
        }
    }
    _game_audit_gc(game, "_game_collect_garbage end");
}

list_t *game_get_group(game_t *game, size_t idx) {
    if (idx < 0 || idx >= game->groups_count) {
        fprintf(stderr,
                "Fatal error: game_get_group: group does not exist: %zu",
                idx);
        exit(1);
    }
    return game->groups[idx];
}

void game_add_body(game_t *game, size_t group_idx, body_t *body) {
    list_add(game_get_group(game, group_idx), body);
}

physics_t *game_get_physics(game_t *game) {
    return game->physics;
}

graphics_t *game_get_graphics(game_t *game) {
    return game->graphics;
}

key_listener_t *game_get_key_listener(game_t *game) {
    return game->key_listener;
}

void *game_get_aux(game_t *game) {
    return game->aux;
}

void _game_timer_free(_game_timer_t *timer) {
    SDL_RemoveTimer(timer->id);
    free(timer);
}

SDL_TimerID game_add_timer(game_t *game,
                           Uint32 interval,
                           SDL_TimerCallback callback,
                           void *aux) {
    _game_timer_t *timer = malloc(sizeof(_game_timer_t));
    timer->removed = false;
    timer->callback = callback;
    timer->aux = aux;
    list_add(game->timers, timer);
    timer->id
        = SDL_AddTimer(interval, (SDL_TimerCallback)_game_callback, timer);
    return timer->id;
}

void game_clear_timers(game_t *game) {
    for (int i = list_size(game->timers) - 1; i >= 0; i--) {
        ((_game_timer_t *)list_get(game->timers, i))->removed = true;
    }
}

Uint32 _game_callback(Uint32 interval, _game_timer_t *timer) {
    timer->interval = interval;
    SDL_Event *event = malloc(sizeof(SDL_Event));
    event->type = SDL_USEREVENT;
    event->user.code = _GAME_EVENT_CODE_TIMER;
    event->user.data1 = timer;
    // event->user->data2 not used, everything put into timer for simplicity.
    event->user.data2 = NULL;
    SDL_PushEvent(event);
    free(event);
    return 0;
}
