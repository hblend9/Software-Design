#include "ball.h"
#include "body.h"
#include "ehhh.h"
#include "forces.h"
#include "game.h"
#include "gfx_aux.h"
#include "list.h"
#include "player.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "sprite.h"
#include "vector.h"
#include <SDL2/SDL_timer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const double POWERUP_ANGLE_INCR = M_PI / 6.0;
const char *BALL_PATH_COLLISION_SHAPE = "static/ball/ball_collision_shape.csv";
const double BALL_MASS = 10;
const double BALL_SHOOT_SPEED = 300;
const size_t FIFTEEN_SECM = 15 * THOUSAND;
const size_t TEN_SECM = 10 * THOUSAND;

const char *POWER_STRS[POWER_NUM_TYPES] = {
    "ball",        // POWER_NONE,
    "ball",        // POWER_PLAYER_EAT_KILL
    "ball",        // POWER_PLAYER_EAT_FAST,
    "ball",        // POWER_PLAYER_EAT_SLOW,
    "ball",        // POWER_PLAYER_EAT_SPILL,
    "faster",      // POWER_PLAYER_ACTIVATE_FAST
    "wider",       // POWER_PLAYER_ACTIVATE_MORE_ANGLE,
    "all:widest",  // POWER_PLAYER_ACTIVATE_WORLD_FULL_ANGLE,
    "all:wider",   // POWER_PLAYER_ACTIVATE_WORLD_MORE_ANGLE,
    "world:reset", // POWER_PLAYER_ACTIVATE_WORLD_RESET_ARENA,
    "all:fast",    // POWER_PLAYER_ACTIVATE_WORLD_FAST,
    "kill",        // POWER_PLAYER_SHOOT_KILL
    "slow",        // POWER_PLAYER_SHOOT_SLOW,
    "spill"        // POWER_PLAYER_SHOOT_SPILL,
};

/*** Custom Function Pointers ***/

typedef void (*powerup_eat_func_t)(ehhh_t *ehhh, player_t *player);
typedef void (*powerup_activate_func_t)(ehhh_t *ehhh,
                                        player_t *player,
                                        ball_power_type_e activate_type);

/*** Struct Definition with Private Fields***/

typedef struct powerup {
    // general powerup type
    ball_power_type_e type; // set on init
    // determines what the spawn type on activate/shoot is
    ball_power_type_e shoot_type; // set on init

    char *sprite_path; // set on init

    powerup_eat_func_t on_eat;           // set on init
    powerup_activate_func_t on_activate; // set on init

    // the value of a ball containing this powerup
    short points; // set on init
} powerup_t;

/*** Private Function Prototypes ***/
void collision_handler_player_ball(body_t *body1,
                                   body_t *body2,
                                   vector_t axis,
                                   vector_t collision_point,
                                   ehhh_t *aux);

void powerup_player_rm_points(player_t *player, powerup_t *powerup);

void powerup_eat_kill(ehhh_t *ehhh, player_t *player);
void powerup_eat_fast(ehhh_t *ehhh, player_t *player);
void powerup_eat_slow(ehhh_t *ehhh, player_t *player);
void powerup_eat_spill(ehhh_t *ehhh, player_t *player);

void powerup_activate_fast(ehhh_t *ehhh,
                           player_t *player,
                           ball_power_type_e activate_type);
void powerup_activate_more_angle(ehhh_t *ehhh,
                                 player_t *player,
                                 ball_power_type_e activate_type);
void powerup_activate_world_full_angle(ehhh_t *ehhh,
                                       player_t *player,
                                       ball_power_type_e activate_type);
void powerup_activate_world_more_angle(ehhh_t *ehhh,
                                       player_t *player,
                                       ball_power_type_e activate_type);
void powerup_activate_world_reset_arena(ehhh_t *ehhh,
                                        player_t *player,
                                        ball_power_type_e activate_type);
void powerup_activate_world_fast(ehhh_t *ehhh,
                                 player_t *player,
                                 ball_power_type_e activate_type);

void powerup_activate_spawn_ball(ehhh_t *ehhh,
                                 player_t *player,
                                 ball_power_type_e activate_type);

Uint32 powerup_reset_fast_callback(Uint32 interval, player_t *player);
Uint32 powerup_reset_slow_callback(Uint32 interval, player_t *player);
Uint32 powerup_undo_angle_callback(Uint32 interval, player_t *player);

/*** Public Function Definitions ***/

char *powerup_str(powerup_t *powerup) {
    if (powerup == NULL) {
        return calloc(1, sizeof(char));
    }
    const char *fmt = "%s>%s";
    const char *class_str;
    const char *power_str;
    const ball_power_type_e pu_type = powerup->type;
    if (pu_type >= POWER_PLAYER_ACTIVATE_LO
        && pu_type < POWER_PLAYER_ACTIVATE_HI) {
        class_str = "activate";
    } else {
        class_str = "shoot";
    }
    power_str = POWER_STRS[pu_type];
    assert(class_str != NULL && power_str != NULL);
    char *s = calloc(strlen(fmt) + strlen(class_str) + strlen(power_str) + 2,
                     sizeof(char));
    if (sprintf(s, fmt, class_str, power_str) < 0) {
        fprintf(stderr, "ball.c: powerup_str: Error calling sprintf.\n");
        exit(1);
    }
    return s;
}

void spawn_ball(ehhh_t *ehhh,
                vector_t init_pos,
                vector_t init_vel,
                ball_power_type_e type) {
    powerup_t *powerup = powerup_init(type);

    sprite_t *sprite = sprite_init(powerup->sprite_path, 1.0 / 20.0, VEC_ZERO);
    list_t *sprites = list_init(1, (free_func_t)sprite_free);
    list_add(sprites, sprite);
    list_t *shapes = list_init(1, (free_func_t)list_free);

    list_t *b = polygon_init_from_path(BALL_PATH_COLLISION_SHAPE);
    polygon_scale(b, 1.0 / 10.0);
    list_add(shapes, b);

    gfx_aux_t *gfx = gfx_aux_init(sprites);

    body_t *ball = body_init_with_gfx(BALL_MASS,
                                      powerup,
                                      (free_func_t)powerup_free,
                                      shapes,
                                      gfx);

    body_set_centroid(ball, init_pos);
    body_set_velocity(ball, init_vel);

    // Create collisions with the ball.
    list_t *players = ehhh_get_players(ehhh);
    list_t *balls = ehhh_get_balls(ehhh);
    physics_t *physics = game_get_physics(ehhh_get_game(ehhh));
    size_t n_ps = list_size(players);
    size_t n_balls = list_size(balls);
    player_t *player = NULL;
    body_t *body = NULL;
    list_t **shape_body_p = NULL;
    for (size_t i = 0; i < n_ps; i++) {
        body = list_get(players, i);
        player = body_get_info(body);
        shape_body_p = body_get_shape_main_p(body);
        // Bounce collisions between players and ball.
        create_physics_collision_shapes(physics,
                                        ehhh_get_elasticity(ehhh),
                                        body,
                                        ball,
                                        shape_body_p,
                                        NULL);
        // Eat collisions between players and ball.
        create_collision_shapes(
            physics,
            body,
            ball,
            player_get_hippo_mouth_shape(player),
            body_get_shape_main_p(ball),
            (collision_handler_t)collision_handler_player_ball,
            ehhh,
            NULL);
    }
    // Bounce collisiosn between balls and ball.
    for (size_t j = 0; j < n_balls; j++) {
        body_t *curr_ball = (body_t *)list_get(balls, j);
        create_physics_collision(physics,
                                 ehhh_get_elasticity(ehhh),
                                 curr_ball,
                                 ball);
    }
    list_add(balls, ball);
}

void powerup_free(powerup_t *powerup) {
    free(powerup);
}

void powerup_activate(powerup_t *powerup, ehhh_t *ehhh, player_t *player) {
    if (powerup->on_activate != NULL) {
        powerup->on_activate(ehhh, player, powerup->shoot_type);
    }
    // if player uses a powerup, remove the points that it gave them
    powerup_player_rm_points(player, powerup);
    // player should remove the powerup when this is called
}

// to be called by the custom collision handler
void powerup_eat(powerup_t *powerup, ehhh_t *ehhh, player_t *player) {
    if (powerup->on_eat != NULL) {
        powerup->on_eat(ehhh, player);
    }
    player_add_powerup(player, powerup);
    player_add_points(player, powerup->points);
}

/*** Private Function Definitions ***/

void collision_handler_player_ball(body_t *body1,
                                   body_t *body2,
                                   vector_t axis,
                                   vector_t collision_point,
                                   ehhh_t *aux) {
    // assume body1 is the player (hippo)
    // assume body2 is the ball

    ehhh_t *ehhh = aux;
    player_t *player = (player_t *)body_get_info(body1);
    powerup_t *powerup = (powerup_t *)body_get_info(body2);

    if (player_is_eating(player)) {
        powerup_eat(powerup, ehhh, player);
        // Remove powerup from the body so it doesn't get freed twice.
        body_set_info(body2, NULL, NULL);
        body_remove(body2);
    } else {
    }
}

powerup_t *powerup_init(ball_power_type_e type) {
    powerup_t *p = malloc(sizeof(powerup_t));

    ball_power_type_e shoot_type = POWER_NONE;

    powerup_eat_func_t on_eat = NULL;
    // by default want to spawn a generic ball
    powerup_activate_func_t on_activate = NULL;

    char *sprite_path = NULL;

    short points = 1;

    switch (type) {
    // generic
    case (POWER_NONE):
        on_eat = NULL;
        on_activate = powerup_activate_spawn_ball;
        shoot_type = POWER_NONE;
        sprite_path = "static/ball/default.png";
        points = 1;
        break;

    // eat types
    case (POWER_PLAYER_EAT_KILL):
        on_eat = powerup_eat_kill;
        on_activate = powerup_activate_spawn_ball;
        shoot_type = POWER_NONE;
        sprite_path = "static/ball/eat_kill.png";
        points = 0;
        break;
    case (POWER_PLAYER_EAT_FAST):
        on_eat = powerup_eat_fast;
        on_activate = powerup_activate_spawn_ball;
        shoot_type = POWER_NONE;
        sprite_path = "static/ball/eat_fast.png";
        points = 1;
        break;
    case (POWER_PLAYER_EAT_SLOW):
        on_eat = powerup_eat_slow;
        on_activate = powerup_activate_spawn_ball;
        shoot_type = POWER_NONE;
        sprite_path = "static/ball/eat_slow.png";
        points = 2;
        break;
    case (POWER_PLAYER_EAT_SPILL):
        on_eat = powerup_eat_spill;
        on_activate = powerup_activate_spawn_ball;
        shoot_type = POWER_NONE;
        sprite_path = "static/ball/eat_spill.png";
        points = -10;
        break;

    // activate types
    case (POWER_PLAYER_ACTIVATE_FAST):
        on_activate = powerup_activate_fast;
        sprite_path = "static/ball/activate_fast.png";
        // higher points means tradeoff between keeping it or using
        points = 3;
        break;
    case (POWER_PLAYER_ACTIVATE_MORE_ANGLE):
        on_activate = powerup_activate_more_angle;
        sprite_path = "static/ball/activate_more_angle.png";
        points = 4;
        break;
    case (POWER_PLAYER_ACTIVATE_WORLD_FULL_ANGLE):
        on_activate = powerup_activate_world_full_angle;
        sprite_path = "static/ball/missing_sprite.png";
        points = 3;
        break;
    case (POWER_PLAYER_ACTIVATE_WORLD_MORE_ANGLE):
        on_activate = powerup_activate_world_more_angle;
        sprite_path = "static/ball/missing_sprite.png";
        points = 3;
        break;
    case (POWER_PLAYER_ACTIVATE_WORLD_RESET_ARENA):
        on_activate = powerup_activate_world_reset_arena;
        sprite_path = "static/ball/missing_sprite.png";
        points = 1;
        break;
    case (POWER_PLAYER_ACTIVATE_WORLD_FAST):
        on_activate = powerup_activate_world_fast;
        sprite_path = "static/ball/missing_sprite.png";
        points = 2;
        break;

        // shoot types
    case (POWER_PLAYER_SHOOT_KILL):
        on_activate = powerup_activate_spawn_ball;
        shoot_type = POWER_PLAYER_EAT_KILL;
        sprite_path = "static/ball/shoot_kill.png";
        points = 8;
        break;
    case (POWER_PLAYER_SHOOT_SLOW):
        on_activate = powerup_activate_spawn_ball;
        shoot_type = POWER_PLAYER_EAT_SLOW;
        sprite_path = "static/ball/shoot_slow.png";
        points = 2;
        break;
    case (POWER_PLAYER_SHOOT_SPILL):
        on_activate = powerup_activate_spawn_ball;
        shoot_type = POWER_PLAYER_EAT_SPILL;
        sprite_path = "static/ball/shoot_spill.png";
        points = 5;
        break;

    // default
    default:
        on_eat = NULL;
        on_activate = NULL;
        shoot_type = POWER_NONE;
        sprite_path = "static/ball/default.png";
        points = 1;
        break;
    }

    p->type = type;
    p->shoot_type = shoot_type;
    p->on_eat = on_eat;
    p->on_activate = on_activate;
    p->sprite_path = sprite_path;
    p->points = points;

    return p;
}

// Not fully implemented, do not call.
void powerup_eat_kill(ehhh_t *ehhh, player_t *player) {
    player_add_points(player, -1 * player_get_points(player));

    powerup_t *powerup = player_get_curr_powerup(player);
    while (powerup != NULL) {
        powerup_free(powerup);
        powerup = player_get_curr_powerup(player);
    }

    /* player_remove(player); */
}

void powerup_eat_fast(ehhh_t *ehhh, player_t *player) {
    player_add_speed_factor(player, 0.5);
    game_add_timer(ehhh_get_game(ehhh),
                   TEN_SECM,
                   (SDL_TimerCallback)powerup_reset_fast_callback,
                   player);
}
void powerup_eat_slow(ehhh_t *ehhh, player_t *player) {
    player_set_speed_factor(player, 0.25);
    game_add_timer(ehhh_get_game(ehhh),
                   FIFTEEN_SECM,
                   (SDL_TimerCallback)powerup_reset_slow_callback,
                   player);
}

// Not fully implemented.
void powerup_eat_spill(ehhh_t *ehhh, player_t *player) {
    powerup_t *powerup = player_get_curr_powerup(player);
    size_t removed = 0;
    while (powerup != NULL) {
        removed++;

        spawn_ball(ehhh,
                   player_get_shoot_location(player, ehhh),
                   vec_rotate(player_get_shoot_velocity(player, ehhh),
                              removed * M_PI / 5.0),
                   powerup->type);
        powerup_player_rm_points(player, powerup);
        powerup_free(powerup);

        // we have removed 5 powerups
        if (removed > 4) {
            powerup = NULL;
        } else {
            // don't remove powerup if we have already removed 5
            powerup = player_get_curr_powerup(player);
        }
    }
}

void powerup_activate_fast(ehhh_t *ehhh,
                           player_t *player,
                           ball_power_type_e activate_type) {
    if (player_get_speed_factor(player) < 1.0) {
        player_set_speed_factor(player, 1.0);
    }
    powerup_eat_fast(ehhh, player);
}
void powerup_activate_more_angle(ehhh_t *ehhh,
                                 player_t *player,
                                 ball_power_type_e activate_type) {
    player_add_angle_modifier(player, POWERUP_ANGLE_INCR);
    game_add_timer(ehhh_get_game(ehhh),
                   FIFTEEN_SECM,
                   (SDL_TimerCallback)powerup_undo_angle_callback,
                   player);
}
void powerup_activate_world_full_angle(ehhh_t *ehhh,
                                       player_t *player,
                                       ball_power_type_e activate_type) {
    list_t *players = ehhh_get_players(ehhh);
    for (size_t i = 0; i < list_size(players); i++) {
        player_set_angle_modifier(list_get(players, i), 2.1 * M_PI);
    }
}
void powerup_activate_world_more_angle(ehhh_t *ehhh,
                                       player_t *player,
                                       ball_power_type_e activate_type) {
    list_t *players = ehhh_get_players(ehhh);
    for (size_t i = 0; i < list_size(players); i++) {
        player_add_angle_modifier(list_get(players, i), POWERUP_ANGLE_INCR);
    }
}

// this will be rather disasterous, please don't
// DEFINITLEY NOT IMPLEMENTED
void powerup_activate_world_reset_arena(ehhh_t *ehhh,
                                        player_t *player,
                                        ball_power_type_e activate_type) {
    assert(false && "deprecated");
}
void powerup_activate_world_fast(ehhh_t *ehhh,
                                 player_t *player,
                                 ball_power_type_e activate_type) {
    ehhh_set_max_speed(ehhh, 1.75);
}

void powerup_activate_spawn_ball(ehhh_t *ehhh,
                                 player_t *player,
                                 ball_power_type_e activate_type) {
    vector_t init_pos = player_get_shoot_location(player, ehhh);
    vector_t init_vel = player_get_shoot_velocity(player, ehhh);
    spawn_ball(ehhh, init_pos, init_vel, activate_type);
}

void powerup_player_rm_points(player_t *player, powerup_t *powerup) {
    if (powerup->points > -1) {
        player_add_points(player, -powerup->points);
    }
}

Uint32 powerup_reset_fast_callback(Uint32 interval, player_t *player) {
    if (player_get_speed_factor(player) > 1.0) {
        player_add_speed_factor(player, -0.5);
    }
    return 0;
}

Uint32 powerup_reset_slow_callback(Uint32 interval, player_t *player) {
    if (player_get_speed_factor(player) < 1.0) {
        player_set_speed_factor(player, 1.0);
    }
    return 0;
}

Uint32 powerup_undo_angle_callback(Uint32 interval, player_t *player) {
    if (player_get_angle_modifier(player) > 0) {
        player_add_angle_modifier(player, -POWERUP_ANGLE_INCR);
    }
    return 0;
}
