#ifndef __GAME_H__
#define __GAME_H__

#include <SDL2/SDL_timer.h>
#include <stdbool.h>
#include <stdlib.h>

/*** DEPENDENCY FORWARD DECLARATIONS ***/
typedef struct body body_t;
typedef struct physics physics_t;
typedef struct graphics graphics_t;
typedef struct vector vector_t;
typedef struct list list_t;
typedef void (*free_func_t)(void *);
typedef struct key_listener key_listener_t;

/**
 * The state of a single entire game. It contains:
 * - Bodies of various groups, which are indexed but the client should enumerate
 *   these indices.
 * - A physics layer.
 * - A graphical layer.
 * - A tick_func_t.
 */
typedef struct game game_t;

/**
 * A function that handles the game's end or round-incrementing behavior.
 * It must return true if the game is completely over or false otherwise.
 * Importantly, just as with everything else, tick_func should not actually free
 * anything itself, rather it should mark things (everything, if it wants) for
 * removal to be freed internally when safe. In particular, all garbage
 * collection is done at the very end of the tick, even after tick_func. Though
 * tick_func can remove everything if it wants, it doesn't need to because
 * game_free will free anything not yet freed, so calling game_free at the very
 * end of your program is sufficient (and necessary!). To pass tick_func
 * anything auxiliary, pass it as 'aux' to game_init and access it via
 * 'game_get_aux' from within the tick_func.
 */
typedef bool (*tick_func_t)(game_t *);

/**
 * Initialize a game with a fixed number of body groups, physics and graphics
 * layers, and a tick_func_t. Body groups, physics, and graphics are initialized
 * empty. You should setup the physics and graphics layers yourself, i.e. add
 * the groups that you want in physics and graphics from game_get_group.  All
 * bodies should be removed via 'body_remove' and game will take care of garbage
 * control. For this reason, body should be considered the top-most object: all
 * other objects should be composed into body via its info field, so that
 * deferred garbage control is not broken.
 * Game is a singleton, so initializing multiple games results in a fatal error.
 * SDL is initialized here.
 */
game_t *game_init(vector_t dims,
                  size_t groups_count,
                  tick_func_t tick_func,
                  void *aux,
                  free_func_t aux_freer);

/**
 * Free a game, its physics and graphics layers, and any bodies that have not
 * yet been removed, along with the aux.
 */
void game_free(game_t *game);

/**
 * Tick the game forward dt seconds.
 * No objects should be marked for removal at the beginning of the tick.
 * Throughout the tick, if something wants to remove an object, it should mark
 * it for removal. Garbage collection is done at the *very* end of the tick, and
 * again nothing done by the client should actually free bodies, that is handled
 * internally. Moreover, if you choose (hint: don't!) to do additional things to
 * the game in your main loop, make sure to call this function at the *end* of
 * the iteration. If you really really want to do more things than are supported
 * by physics and graphics layers, then put them in your tick_func, dumb punk!
 * The sdl event queue is emptied and handled each tick via the key handler and
 * callbacks; you shouldn't poll events yourself. The custom tick_func is called
 * just before game's garbage removal, so it should do any auxiliary garbage
 * removal at its very end.
 * @return true if the game is over, false otherwise.
 */
bool game_tick(game_t *game, double dt);

/**
 * Get the list of bodies in group by index.
 * This list will be updated each tick to only contain the bodies not marked for
 * removal. The client should not manually remove from or free this list, as
 * that will be taken care of by game.
 * The client really ought to enumerate the group indices.
 * Trying to access a group that does not exist results in a fatal error.
 */
list_t *game_get_group(game_t *game, size_t idx);

/**
 * Add a body to a group.
 */
void game_add_body(game_t *game, size_t group_idx, body_t *body);

/**
 * Get the game's physics layer.
 */
physics_t *game_get_physics(game_t *game);

/**
 * Get the game's graphics layer.
 */
graphics_t *game_get_graphics(game_t *game);

/**
 * Get the game's key listener.
 */
key_listener_t *game_get_key_listener(game_t *game);

void *game_get_aux(game_t *game);

/**
 * Add a callback timer and return its id.
 *
 * Our timer mechanism wraps SDL's timer so as to avoid multithreading issues
 * (in perhaps a suboptimal way at the cost of speed, but I don't have time to
 * make it any better). Therefore, feel free to call arbitrary functions inside
 * of the callback, and they will be executed in the main thread.
 *
 * The game removes all timers on game_free, but it is your responsibility to
 * make sure the callbacks don't get removed after something is marked for
 * removal, because garbage could be collected at any point after something is
 * marked for removal.
 *
 * The aux is not freed by game, so you should.
 */
SDL_TimerID game_add_timer(game_t *game,
                           Uint32 interval,
                           SDL_TimerCallback callback,
                           void *aux);

/**
 * Cancel all timers registered with the game.
 * As with game_free, timer auxs are not freed. Timers are marked for deferred
 * removal and will not be called if they show up on the event queue hereafter.
 */
void game_clear_timers(game_t *game);

#endif // #ifndef __GAME_H__
