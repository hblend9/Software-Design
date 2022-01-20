#include "ehhh.h"
#include "boundary.h"
#include "game.h"
#include "graphics.h"
#include "key_listener.h"
#include "list.h"
#include "movement.h"
#include "physics.h"
#include "player.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "shapes_geometry.h"
#include "text.h"
#include "vector.h"
#include "wrand.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <time.h>

/*** TYPES ***/

struct ehhh {
    game_t *game;
    list_t *text_tabs;
    list_t *text_lns;
    list_t *countdown_auxs;
    size_t ball_count_round; // Number of balls created this round.
    size_t max_ball_round_count;
    double elasticity;
    double max_speed;
    double max_angle;
    vector_t center;
    wrand_t *wrand_ball_type;
};

typedef struct _ehhh_player_act_aux _ehhh_player_act_aux_t;
typedef void (*_ehhh_player_act_func_t)(_ehhh_player_act_aux_t *);
struct _ehhh_player_act_aux {
    ehhh_t *ehhh;
    player_t *player;
    _ehhh_player_act_func_t player_act;
    bool released;
};

typedef struct _ehhh_countdown_aux {
    ehhh_t *ehhh;
    text_ln_t *text_ln;
    int count;
    void (*func)(ehhh_t *ehhh);
    bool removed;
} _ehhh_countdown_aux_t;

typedef enum _ehhh_group {
    _EHHH_GROUP_BACKGROUND,
    _EHHH_GROUP_PLAYER,
    _EHHH_GROUP_BALL,
    _EHHH_GROUP_COUNT
} _ehhh_group_t;

/*** CONSTANTS
 * All game params are defined here until they need to be un-factored into main
 * for the purpose of being passed via the command line.
 ***/

// Paths
const char *_EHHH_BACKGROUND_SPRITE_PATH = "static/background-sprite.png";

// Dimensions
const double _EHHH_BACKGROUND_SCALE = 1.0 / 2.0;
#define _EHHH_MAX_X 2000
#define _EHHH_MAX_Y 1000
const vector_t _EHHH_DIMS = {_EHHH_MAX_X, _EHHH_MAX_Y};
const double _EHHH_WORLD_RADIUS = 450.0;
const double _EHHH_PLAYER_OFFSET = 100; // Offset outside of radius.
const double _EHHH_BALL_RADIUS = 33.0;
const vector_t _EHHH_PLAYER_TAB_POSITION = {30, 800};
const double _EHHH_BUFFER_ZONE = 2000;
const double _EHHH_MAX_ANGLE_BUFFER = M_PI / 15; // Subtracted from raw max.

// Keys
typedef enum _ehhh_player_act {
    _EHHH_PLAYER_ACT_LEFT,
    _EHHH_PLAYER_ACT_RIGHT,
    _EHHH_PLAYER_ACT_FORWARD,
    _EHHH_PLAYER_ACT_ACTIVATE,
    _EHHH_PLAYER_ACT_SWITCH, // Increment powerup (dw, over finite field).
    _EHHH_PLAYER_ACT_COUNT
} _ehhh_player_act_t;
void _ehhh_player_act_right(_ehhh_player_act_aux_t *aux);
void _ehhh_player_act_left(_ehhh_player_act_aux_t *aux);
void _ehhh_player_act_forward(_ehhh_player_act_aux_t *aux);
void _ehhh_player_act_activate(_ehhh_player_act_aux_t *aux);
void _ehhh_player_act_switch(_ehhh_player_act_aux_t *aux);
const _ehhh_player_act_func_t _EHHH_PLAYER_ACT_TO_FUNC[_EHHH_PLAYER_ACT_COUNT]
    = {_ehhh_player_act_left,
       _ehhh_player_act_right,
       _ehhh_player_act_forward,
       _ehhh_player_act_activate,
       _ehhh_player_act_switch};
const SDL_Keycode _EHHH_PLAYER_KEYS[EHHH_MAX_PLAYERS][_EHHH_PLAYER_ACT_COUNT]
    = {/*LEFT    , RIGHT     , FORWARD  , ACTIVATE , SWITCH*/
       {SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_DOWN, SDLK_SLASH},
       {SDLK_a, SDLK_d, SDLK_w, SDLK_s, SDLK_e},
       {SDLK_h, SDLK_l, SDLK_k, SDLK_j, SDLK_i},
       {SDLK_KP_1, SDLK_KP_3, SDLK_KP_5, SDLK_KP_2, SDLK_KP_4}};

// Dynamics
const Uint32 _EHHH_INTERVAL_NOW = 2; // Delay for immediate callback.
const Uint32 _EHHH_INTERVAL = 10;    // Default milliseconds between callbacks.
#define _EHHH_SPEED_AMPLIFICATION_FACTOR 100
const double _EHHH_BALL_SPEED = 3 * _EHHH_SPEED_AMPLIFICATION_FACTOR;
const double _EHHH_ELASTICITY = 1.0;
const double _EHHH_PLAYER_SIDE_SPEED = 0.005;         // Per millisecond.
const double _EHHH_PLAYER_FORWARD_FACTOR = 1.0 / 3.0; // (Deprecated)
const double _EHHH_PLAYER_EAT_TIME = 250;             // Milliseconds.
const double _EHHH_PLAYER_FORWARD_TIME = 150; // Milliseconds. (Deprecated)

// Balls
const size_t EHHH_MAX_BALLS_PER_ROUND = 50;
const Uint32 _EHHH_BALL_SPAWN_INTERVAL = 0.2e3; // milliseconds
#define _EHHH_BALL_TYPE_COUNT 6
const ball_power_type_e _EHHH_BALL_TYPES[_EHHH_BALL_TYPE_COUNT]
    = {POWER_NONE,
       POWER_PLAYER_EAT_FAST,
       POWER_PLAYER_EAT_SLOW,
       POWER_PLAYER_ACTIVATE_FAST,
       POWER_PLAYER_ACTIVATE_MORE_ANGLE,
       POWER_PLAYER_SHOOT_SLOW};
const double _EHHH_BALL_TYPE_WEIGHTS[_EHHH_BALL_TYPE_COUNT] = {
    10, // POWER_NONE;
    1,  // POWER_PLAYER_EAT_FAST;
    2,  // POWER_PLAYER_EAT_SLOW;
    1,  // POWER_PLAYER_ACTIVATE_FAST;
    3,  // POWER_PLAYER_ACTIVATE_MORE_ANGLE;
    2,  // POWER_PLAYER_SHOOT_SLOW;
};

// Text.
const int _EHHH_SPLASH_HEIGHT = 40;
const text_style_t _EHHH_SPLASH_STYLE = TEXT_STYLE_BOLD;
const SDL_Color _EHHH_SPLASH_COLOR = {0x00, 0x2b, 0x36, 0xfa};
const Uint32 _EHHH_INTERVAL_SPLASH_LONG = 4e3; // Milliseconds.
const Uint32 _EHHH_INTERVAL_COUNTDOWN = 1e3;   // milliseconds

/*** PRIVATE PROTOTYPES ***/

/**
 * Determine whether the game is over or needs to be incremented one round.
 * If the latter, then do so.
 * Return true of the game is over, false otherwise.
 */
bool _ehhh_tick(game_t *game);

/**
 * Initialize players and their bodies.
 */
void _ehhh_init_players(ehhh_t *ehhh, size_t player_count);

/**
 * Initialize a single player and its body and return it, but do not add it to
 * the game yet.
 */
body_t *_ehhh_init_player(ehhh_t *ehhh,
                          size_t pos_idx,
                          size_t player_idx,
                          size_t player_count);

/**
 * Initialize balls.
 */
void _ehhh_init_ball_spawning(ehhh_t *ehhh);

/**
 * Initialize the area background.
 */
void _ehhh_init_background(ehhh_t *ehhh);

/**
 * Compute player positions.
 */
vector_t
_ehhh_player_position(ehhh_t *ehhh, size_t player_count, size_t pos_idx);

/**
 * Spawn balls at random angles each tick.
 */
void _ehhh_spawn_ball_random_angle(ehhh_t *ehhh);

Uint32 _ehhh_spawn_ball_callback(Uint32 interval, ehhh_t *ehhh);

void _ehhh_enforce_boundary(ehhh_t *ehhh);

void _ehhh_setup_player_keys(ehhh_t *ehhh, list_t *player_bodies);
void _ehhh_player_on_key(SDL_KeyboardEvent event, _ehhh_player_act_aux_t *aux);

Uint32 _ehhh_player_back_callback(Uint32 interval, _ehhh_player_act_aux_t *aux);
Uint32 _ehhh_player_chill_callback(Uint32 interval,
                                   _ehhh_player_act_aux_t *aux);
Uint32 _ehhh_player_right_callback(Uint32 interval,
                                   _ehhh_player_act_aux_t *aux);
Uint32 _ehhh_player_left_callback(Uint32 interval, _ehhh_player_act_aux_t *aux);

/**
 * Increment round or trigger game-over if appropriate.
 */
bool _ehhh_handle_rounds(ehhh_t *ehhh);

/**
 * Return the looser's body.
 */
body_t *_ehhh_find_loser(ehhh_t *ehhh);

/**
 * Return whether the win condition has been met, but do nothing to the game.
 */
bool _ehhh_win_condition(ehhh_t *ehhh);

/**
 * Return whether the game-over condition has been met, assuming that the win
 * condition has already been met, but do nothing to the game.
 */
bool _ehhh_end_condition(ehhh_t *ehhh);

/**
 * Return a fresh list of player bodies who were not marked for removal before
 * the call. Mark every existing player body for removal, then create a fresh
 * set of bodies for those players who weren't marked for removal, preserving
 * their player idx.
 */
list_t *_ehhh_refresh_players(ehhh_t *ehhh, list_t *winners_old);

/**
 * Deferred removal.
 */
void _ehhh_collect_garbage(ehhh_t *ehhh);

void _ehhh_init_round_sequence(ehhh_t *ehhh, char *s);
void _ehhh_incr_round(ehhh_t *ehhh, body_t *loser);
void _ehhh_init_end_sequence(ehhh_t *ehhh, body_t *winner, short points);

Uint32 _ehhh_start_round_callback(Uint32 interval, _ehhh_countdown_aux_t *aux);
Uint32 _ehhh_countdown_callback(Uint32 interval, _ehhh_countdown_aux_t *aux);
Uint32 _ehhh_game_over_callback(Uint32 interval, text_ln_t *aux);

/*** DEFINITIONS ***/

ehhh_t *ehhh_init(size_t player_count, size_t balls_per_round) {
    // Constants that may some day be passed from main but for now are here.
    double elasticity = _EHHH_ELASTICITY;
    vector_t dims = _EHHH_DIMS;
    size_t max_ball_round_count = balls_per_round;

    if (player_count > EHHH_MAX_PLAYERS) {
        fprintf(stderr,
                "Fatal error: maximum of %d players exceeded.\n",
                EHHH_MAX_PLAYERS);
        exit(1);
    }
    if (player_count < EHHH_MIN_PLAYERS) {
        fprintf(stderr,
                "Fatal error: minimum of %d players exceeded.\n",
                EHHH_MIN_PLAYERS);
    }

    srand(time(NULL));

    ehhh_t *ehhh = malloc(sizeof(ehhh_t));
    assert(ehhh != NULL);
    ehhh->countdown_auxs = list_init(1, free);

    game_t *game = ehhh->game
        = game_init(dims, _EHHH_GROUP_COUNT, _ehhh_tick, ehhh, NULL);
    ehhh->elasticity = elasticity;
    ehhh->center = vec_multiply(1.0 / 2.0, dims);
    ehhh->wrand_ball_type
        = wrand_init(_EHHH_BALL_TYPE_COUNT, _EHHH_BALL_TYPE_WEIGHTS);
    ehhh->max_ball_round_count = max_ball_round_count;

    // Setup physics and graphics with correct bodies.
    physics_add_bodies(game_get_physics(game), ehhh_get_players(ehhh));
    physics_add_bodies(game_get_physics(game), ehhh_get_balls(ehhh));
    graphics_add_bodies(game_get_graphics(game),
                        game_get_group(game, _EHHH_GROUP_BACKGROUND));
    graphics_add_bodies(game_get_graphics(game), ehhh_get_players(ehhh));
    graphics_add_bodies(game_get_graphics(game), ehhh_get_balls(ehhh));

    // Setup bodies.
    _ehhh_init_background(ehhh);
    _ehhh_init_players(ehhh, player_count);
    ehhh->ball_count_round = 0;
    // Ball spawning are started by _ehhh_init_round_sequence.

    // Setup text.
    list_t *tabs = ehhh->text_tabs = list_init(1, (free_func_t)text_tab_free);
    list_add(tabs,
             player_create_tab(game_get_group(game, _EHHH_GROUP_PLAYER),
                               _EHHH_PLAYER_TAB_POSITION));
    graphics_add_text_tabs(game_get_graphics(game), tabs);
    ehhh->text_lns = list_init(1, (free_func_t)text_ln_free);
    graphics_add_text_lns(game_get_graphics(game), ehhh->text_lns);

    // Setup keys.
    _ehhh_setup_player_keys(ehhh, ehhh_get_players(ehhh));

    // Start the first round.
    _ehhh_init_round_sequence(ehhh, "YOU ARE A HUNGRY HIPPO");

    return ehhh;
}

bool ehhh_tick(ehhh_t *ehhh, double dt) {
    return game_tick(ehhh->game, dt);
}

void ehhh_free(ehhh_t *ehhh) {
    game_free(ehhh->game);
    list_free(ehhh->text_tabs);
    list_free(ehhh->text_lns);
    list_free(ehhh->countdown_auxs);
    wrand_free(ehhh->wrand_ball_type);
    free(ehhh);
}

bool _ehhh_tick(game_t *game) {
    ehhh_t *ehhh = game_get_aux(game);
    _ehhh_enforce_boundary(ehhh);
    bool done = _ehhh_handle_rounds(ehhh);
    _ehhh_collect_garbage(ehhh);
    return done;
}

void _ehhh_collect_garbage(ehhh_t *ehhh) {
    for (int i = list_size(ehhh->text_lns) - 1; i >= 0; i--) {
        if (text_ln_is_removed(list_get(ehhh->text_lns, i))) {
            text_ln_free(list_remove(ehhh->text_lns, i));
        }
    }
    for (int i = list_size(ehhh->countdown_auxs) - 1; i >= 0; i--) {
        if (((_ehhh_countdown_aux_t *)list_get(ehhh->countdown_auxs, i))
                ->removed) {
            free(list_remove(ehhh->countdown_auxs, i));
        }
    }
}

bool _ehhh_win_condition(ehhh_t *ehhh) {
    return ehhh->ball_count_round > 0 && list_size(ehhh_get_balls(ehhh)) <= 0;
}

bool _ehhh_end_condition(ehhh_t *ehhh) {
    return list_size(ehhh_get_players(ehhh)) <= 2;
}

bool _ehhh_handle_rounds(ehhh_t *ehhh) {
    if (_ehhh_win_condition(ehhh)) { // Someone won.
        // We have to do this before we start adding to the player list.
        bool end = _ehhh_end_condition(ehhh);
        // We refresh the players without keys in all cases, so the winner can
        // be displayed at the end.
        body_t *loser = _ehhh_find_loser(ehhh);
        body_remove(loser);
        list_t *winners_old = list_init(1, NULL);
        list_t *winners = _ehhh_refresh_players(ehhh, winners_old);
        game_clear_timers(ehhh->game);
        key_listener_refresh(game_get_key_listener(ehhh->game));
        if (end) {
            assert(list_size(winners_old) == 1);
            assert(list_size(winners) == 1);
            short points
                = player_get_points(body_get_info(list_get(winners_old, 0)));
            _ehhh_init_end_sequence(ehhh, list_get(winners, 0), points);
        } else {
            ehhh->ball_count_round = 0;
            _ehhh_setup_player_keys(ehhh, winners);
            _ehhh_incr_round(ehhh, loser);
        }
        list_free(winners);
        list_free(winners_old);
    }
    return false;
}

void _ehhh_incr_round(ehhh_t *ehhh, body_t *loser) {
    char *fmt = "SAD, HIPPO %1zu HAS BEEN ELIMINATED";
    char *s = calloc(strlen(fmt) + /*idx*/ 1 + /*\0*/ 1, sizeof(char));
    if (sprintf(s, fmt, player_get_idx(body_get_info(loser))) < 0) {
        fprintf(stderr,
                "ehhh.c: _ehhh_init_round_sequence: Error calling sprintf.\n");
        exit(1);
    }
    _ehhh_init_round_sequence(ehhh, s);
    free(s);
}

void _ehhh_init_round_sequence(ehhh_t *ehhh, char *s) {
    char *str = calloc(strlen(s) + 1, sizeof(char));
    strcpy(str, s);
    assert(strlen(str) > 0);
    text_ln_t *ln = ehhh_splash(ehhh, str);
    free(str);
    _ehhh_countdown_aux_t *aux = malloc(sizeof(_ehhh_countdown_aux_t));
    list_add(ehhh->countdown_auxs, aux);
    aux->removed = false;
    aux->ehhh = ehhh;
    aux->text_ln = ln;
    aux->count = 3;
    aux->func = _ehhh_init_ball_spawning;
    game_add_timer(ehhh->game,
                   _EHHH_INTERVAL_SPLASH_LONG,
                   (SDL_TimerCallback)_ehhh_start_round_callback,
                   aux);
}

Uint32 _ehhh_start_round_callback(Uint32 interval, _ehhh_countdown_aux_t *aux) {
    text_ln_update(aux->text_ln, "ROUND STARTING IN...");
    game_add_timer(aux->ehhh->game,
                   _EHHH_INTERVAL_COUNTDOWN,
                   (SDL_TimerCallback)_ehhh_countdown_callback,
                   aux);
    return 0;
}

Uint32 _ehhh_countdown_callback(Uint32 interval, _ehhh_countdown_aux_t *aux) {
    if (aux->count < 0) {
        text_ln_remove(aux->text_ln);
        aux->func(aux->ehhh);
        aux->removed = true;
        return 0;
    } else {
        char *fmt = "%d";
        char *s = calloc(strlen(fmt) + /*count*/ 1 + /*\0*/ 1, sizeof(char));
        if (sprintf(s, fmt, aux->count) < 0) {
            fprintf(
                stderr,
                "ehhh.c: _ehhh_countdown_callback: Error calling sprintf.\n");
            exit(1);
        }
        assert(strlen(s) > 0);
        text_ln_update(aux->text_ln, s);
        free(s);
        aux->count--;
        return _EHHH_INTERVAL_COUNTDOWN;
    }
}

void _ehhh_init_end_sequence(ehhh_t *ehhh, body_t *winner, short points) {
    ehhh->ball_count_round = 0;
    player_add_points(body_get_info(winner), points);
    char *fmt = "VERY COOL, HIPPO %1zu WON THE GAME";
    char *s = calloc(strlen(fmt) + /*idx*/ 1 + /*\0*/ 1, sizeof(char));
    if (sprintf(s, fmt, player_get_idx(body_get_info(winner))) < 0) {
        fprintf(stderr,
                "ehhh.c: _ehhh_init_end_sequence: Error calling sprintf.\n");
        exit(1);
    }
    assert(strlen(s) > 0);
    text_ln_t *ln = ehhh_splash(ehhh, s);
    free(s);
    game_add_timer(ehhh->game,
                   _EHHH_INTERVAL_SPLASH_LONG,
                   (SDL_TimerCallback)_ehhh_game_over_callback,
                   ln);
}

Uint32 _ehhh_game_over_callback(Uint32 interval, text_ln_t *aux) {
    text_ln_update(aux, "THAT IS ALL.");
    return 0;
}

void _ehhh_enforce_boundary(ehhh_t *ehhh) {
    list_t *balls = ehhh_get_balls(ehhh);
    for (int i = 0; i < list_size(balls); i++) {
        body_t *curr_ball = list_get(balls, i);
        boundary_arena_ball_collision(curr_ball,
                                      ehhh->center,
                                      _EHHH_WORLD_RADIUS - _EHHH_BALL_RADIUS,
                                      _EHHH_BALL_RADIUS,
                                      _EHHH_BUFFER_ZONE,
                                      _EHHH_BALL_SPEED);
        reset_boundary_balls(curr_ball,
                             ehhh->center,
                             _EHHH_WORLD_RADIUS,
                             _EHHH_BUFFER_ZONE,
                             _EHHH_BALL_SPEED);
    }
}

void _ehhh_init_ball_spawning(ehhh_t *ehhh) {
    ehhh->ball_count_round = 0;
    game_add_timer(ehhh_get_game(ehhh),
                   _EHHH_INTERVAL_NOW,
                   (SDL_TimerCallback)_ehhh_spawn_ball_callback,
                   ehhh);
}

void _ehhh_init_players(ehhh_t *ehhh, size_t player_count) {
    list_t *group = ehhh_get_players(ehhh);
    for (size_t i = 0; i < player_count; i++) {
        list_add(group, _ehhh_init_player(ehhh, i, i, player_count));
    }
}

body_t *_ehhh_init_player(ehhh_t *ehhh,
                          size_t pos_idx,
                          size_t player_idx,
                          size_t player_count) {
    vector_t pos = _ehhh_player_position(ehhh, player_count, pos_idx);
    player_t *player = player_init(player_idx, pos);
    body_t *body = player_get_body(player);
    double angle_rotate = 2 * M_PI / player_count * pos_idx;
    body_set_rotation(body, angle_rotate);
    player_set_state(player, PLAYER_CHILLING);
    return body;
}

body_t *_ehhh_find_loser(ehhh_t *ehhh) {
    list_t *players = ehhh_get_players(ehhh);
    assert(list_size(players) > 0);
    player_t *loser_player = body_get_info(list_get(players, 0));
    for (size_t i = 0; i < list_size(players); i++) {
        player_t *player = body_get_info(list_get(players, i));
        if (player_get_points(player) < player_get_points(loser_player)) {
            loser_player = player;
        }
    }
    return player_get_body(loser_player);
}

list_t *_ehhh_refresh_players(ehhh_t *ehhh, list_t *winners_old) {
    list_t *players = ehhh_get_players(ehhh);
    assert(winners_old != NULL && list_size(winners_old) == 0);
    list_t *winners_new = list_init(list_size(players), NULL);
    // Find winners, i.e. those not marked for removal.
    for (size_t i = 0; i < list_size(players); i++) {
        if (!body_is_removed(list_get(players, i))) {
            list_add(winners_old, list_get(players, i));
        }
    }
    // Mark all old bodies for removal.
    for (size_t i = 0; i < list_size(players); i++) {
        body_remove(list_get(players, i));
    }
    // Winners get to transcend to fresh bodies.
    body_t *winner_new;
    for (size_t i = 0; i < list_size(winners_old); i++) {
        winner_new = _ehhh_init_player(
            ehhh,
            i,
            player_get_idx(body_get_info(list_get(winners_old, i))),
            list_size(winners_old));
        list_add(players, winner_new);
        list_add(winners_new, winner_new);
    }
    return winners_new;
}

vector_t
_ehhh_player_position(ehhh_t *ehhh, size_t player_count, size_t pos_idx) {
    if ((player_count <= 0) || (player_count > EHHH_MAX_PLAYERS)
        || pos_idx > EHHH_MAX_PLAYERS) {
        assert(false);
    }
    double angle_rotate = 2 * M_PI / player_count * pos_idx;
    vector_t subtract_by
        = {.x = 0.0, .y = _EHHH_WORLD_RADIUS + _EHHH_PLAYER_OFFSET};
    vector_t default_vector = vec_subtract(ehhh->center, subtract_by);
    return vec_rotate_relative(default_vector, angle_rotate, ehhh->center);
}

void _ehhh_init_background(ehhh_t *ehhh) {
    sprite_t *sprite = sprite_init(_EHHH_BACKGROUND_SPRITE_PATH,
                                   _EHHH_BACKGROUND_SCALE,
                                   VEC_ZERO);
    list_t *sprites = list_init(1, (free_func_t)sprite_free);
    list_add(sprites, sprite);
    list_t *shapes = list_init(1, (free_func_t)list_free);
    list_t *background = make_rectangle_polygon(ehhh->center, 5, 5);
    list_add(shapes, background);
    gfx_aux_t *gfx = gfx_aux_init(sprites);
    body_t *background_body
        = body_init_with_gfx(INFINITY, NULL, NULL, shapes, gfx);
    body_set_centroid(background_body, ehhh->center);
    list_add(game_get_group(ehhh->game, _EHHH_GROUP_BACKGROUND),
             background_body);
}

void _ehhh_spawn_ball(ehhh_t *ehhh, vector_t velocity) {
    if (ehhh->ball_count_round >= ehhh->max_ball_round_count) {
        return;
    }
    size_t ball_type_idx = wrand_sample(ehhh->wrand_ball_type);
    assert(ball_type_idx >= 0 && ball_type_idx < _EHHH_BALL_TYPE_COUNT);
    ball_power_type_e type_new_ball = _EHHH_BALL_TYPES[ball_type_idx];
    spawn_ball(ehhh, ehhh->center, velocity, type_new_ball);
    ehhh->ball_count_round++;
}

void _ehhh_spawn_ball_random_angle(ehhh_t *ehhh) {
    double angle_initial_velocity = (rand() / (double)RAND_MAX * M_PI * 2);
    vector_t initial_velocity = vec_rotate(vec_multiply(_EHHH_BALL_SPEED, E1),
                                           angle_initial_velocity);
    _ehhh_spawn_ball(ehhh, initial_velocity);
}

Uint32 _ehhh_spawn_ball_callback(Uint32 interval, ehhh_t *ehhh) {
    _ehhh_spawn_ball_random_angle(ehhh);
    if (ehhh->ball_count_round < ehhh->max_ball_round_count) {
        return _EHHH_BALL_SPAWN_INTERVAL;
    } else {
        return 0;
    }
}

list_t *ehhh_get_players(ehhh_t *ehhh) {
    return game_get_group(ehhh->game, _EHHH_GROUP_PLAYER);
}

list_t *ehhh_get_balls(ehhh_t *ehhh) {
    return game_get_group(ehhh->game, _EHHH_GROUP_BALL);
}

game_t *ehhh_get_game(ehhh_t *ehhh) {
    return ehhh->game;
}

double ehhh_get_elasticity(ehhh_t *ehhh) {
    return ehhh->elasticity;
}

double ehhh_get_max_speed(ehhh_t *ehhh) {
    return ehhh->max_speed;
}

double ehhh_get_max_angle(ehhh_t *ehhh) {
    assert(list_size(ehhh_get_players(ehhh)) > 0);
    return M_PI / ((double)list_size(ehhh_get_players(ehhh)))
           - _EHHH_MAX_ANGLE_BUFFER;
}

void ehhh_set_max_speed(ehhh_t *ehhh, double value) {
    ehhh->max_speed = value;
}

void ehhh_set_max_angle(ehhh_t *ehhh, double value) {
    /* DEPRECATED */
}

vector_t ehhh_get_center(ehhh_t *ehhh) {
    return ehhh->center;
}

double ehhh_get_radius(ehhh_t *ehhh) {
    return _EHHH_WORLD_RADIUS;
}

void _ehhh_setup_player_keys(ehhh_t *ehhh, list_t *player_bodies) {
    assert(list_size(player_bodies) <= EHHH_MAX_PLAYERS);
    key_listener_t *key_listener = game_get_key_listener(ehhh->game);
    player_t *player;
    for (size_t i = 0; i < list_size(player_bodies); i++) {
        player = body_get_info(list_get(player_bodies, i));
        for (size_t j = 0; j < _EHHH_PLAYER_ACT_COUNT; j++) {
            _ehhh_player_act_aux_t *aux
                = malloc(sizeof(_ehhh_player_act_aux_t));
            aux->player = player;
            assert(player_get_idx(aux->player) <= EHHH_MAX_PLAYERS);
            aux->ehhh = ehhh;
            aux->player_act = _EHHH_PLAYER_ACT_TO_FUNC[j];
            aux->released = true;
            key_listener_add(key_listener,
                             (ear_func_t)_ehhh_player_on_key,
                             aux,
                             _EHHH_PLAYER_KEYS[player_get_idx(aux->player)][j],
                             free);
        }
    }
}

void _ehhh_player_on_key(SDL_KeyboardEvent event, _ehhh_player_act_aux_t *aux) {
    aux->released = event.state == SDL_RELEASED;
    // Callback timers will end as necessary when aux->released is set to true.
    // Thus, player_act functions are only responsible for starting the actions.
    if (!aux->released) {
        aux->player_act(aux);
    }
}

void _ehhh_player_act_right(_ehhh_player_act_aux_t *aux) {
    assert(!aux->released);
    game_add_timer(aux->ehhh->game,
                   _EHHH_INTERVAL_NOW,
                   (SDL_TimerCallback)_ehhh_player_right_callback,
                   aux);
}

void _ehhh_player_act_left(_ehhh_player_act_aux_t *aux) {
    assert(!aux->released);
    game_add_timer(aux->ehhh->game,
                   _EHHH_INTERVAL_NOW,
                   (SDL_TimerCallback)_ehhh_player_left_callback,
                   aux);
}

void _ehhh_player_act_forward(_ehhh_player_act_aux_t *aux) {
    player_set_state(aux->player, PLAYER_EATING);
    game_t *game = ehhh_get_game(aux->ehhh);
    // This callback can bring the hippo back before it stops eating,
    // which might make it look more natural but unclear.
    // game_add_timer(game,
    //                _EHHH_PLAYER_FORWARD_TIME,
    //                (SDL_TimerCallback)_ehhh_player_back_callback,
    //                aux);
    game_add_timer(game,
                   _EHHH_PLAYER_EAT_TIME,
                   (SDL_TimerCallback)_ehhh_player_chill_callback,
                   aux);
}

void _ehhh_player_act_activate(_ehhh_player_act_aux_t *aux) {
    player_activate_powerup(aux->player, aux->ehhh);
}

void _ehhh_player_act_switch(_ehhh_player_act_aux_t *aux) {
    player_inc_powerup_idx(aux->player, 1);
}

Uint32 _ehhh_player_back_callback(Uint32 interval,
                                  _ehhh_player_act_aux_t *aux) {
    body_set_shape_main(player_get_body(aux->player),
                        PLAYER_HIPPO_BODY_NOT_EATING_IDX);
    gfx_aux_set_sprite(body_get_gfx(player_get_body(aux->player)),
                       PLAYER_SPRITE_CHILLING);
    return 0;
}

Uint32 _ehhh_player_chill_callback(Uint32 interval,
                                   _ehhh_player_act_aux_t *aux) {
    player_set_state(aux->player, PLAYER_CHILLING);
    return 0;
}

Uint32 _ehhh_player_right_callback(Uint32 interval,
                                   _ehhh_player_act_aux_t *aux) {
    hippo_movement_side(player_get_body(aux->player),
                        ehhh_get_center(aux->ehhh),
                        player_get_origin(aux->player),
                        ehhh_get_radius(aux->ehhh),
                        ehhh_get_max_angle(aux->ehhh)
                            + player_get_angle_modifier(aux->player),
                        player_get_speed_factor(aux->player)
                            * _EHHH_PLAYER_SIDE_SPEED * interval,
                        1.0);
    return aux->released ? 0 : _EHHH_INTERVAL;
}

Uint32 _ehhh_player_left_callback(Uint32 interval,
                                  _ehhh_player_act_aux_t *aux) {
    hippo_movement_side(player_get_body(aux->player),
                        ehhh_get_center(aux->ehhh),
                        player_get_origin(aux->player),
                        ehhh_get_radius(aux->ehhh),
                        ehhh_get_max_angle(aux->ehhh)
                            + player_get_angle_modifier(aux->player),
                        player_get_speed_factor(aux->player)
                            * _EHHH_PLAYER_SIDE_SPEED * interval,
                        -1.0);
    return aux->released ? 0 : _EHHH_INTERVAL;
}

text_ln_t *ehhh_splash(ehhh_t *ehhh, char *str) {
    text_ln_t *ln = text_ln_init(str,
                                 ehhh->center,
                                 _EHHH_SPLASH_HEIGHT,
                                 _EHHH_SPLASH_COLOR,
                                 _EHHH_SPLASH_STYLE);
    list_add(ehhh->text_lns, ln);
    return ln;
}
