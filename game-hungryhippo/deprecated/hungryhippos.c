#include "body.h"
#include "boundary.h"
#include "collision.h"
#include "color.h"
#include "forces.h"
#include "game_world.h"
#include "graphics.h"
#include "key_handling.h"
#include "list.h"
#include "movement.h"
#include "player.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "shapes_geometry.h"
#include "sprite.h"
#include "text.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_X 0.0
#define MIN_Y 0.0
#define MAX_X 2000
#define MAX_Y 1000
const vector_t MIN = {MIN_X, MIN_Y};
const vector_t MAX = {MAX_X, MAX_Y};

const double BUFFER = 10;

const int INIT_NUM_HIPPOS = 4;
const vector_t CENTER_WORLD
    = {.x = (MAX_X - MIN_X) / 2.0, .y = (MAX_Y - MIN_Y) / 2.0};
const double WORLD_RADIUS = 450.0;
const double MAX_ANGLE = 2 * M_PI / 8.0;
const bool SPAWN_POOL_METHOD = false;
const bool SPAWN_SPRINKLER_METHOD = false;
const bool SPAWN_RANDOM_ANGLE_METHOD = true;
const int MAX_BALLS_THAT_FIT_ON_SCREEN = 64;
const double CONTINOUS_SPAWN_TIME_INTERVAL = 0.2; // seconds
const double HIPPO_EAT_TIME = 150;                // milliseconds
const double HIPPO_EAT_WAIT_TIME = 250;
const double BUFFER_ZONE = 200;
const bool SPECIAL_CASE_GAME_OVER = 5;
const double WAIT_SPAWN_BALLS_TIME = 2.0;   // seconds
const double WAIT_RESTART_ROUND_TIME = 2.0; // seconds

#define SPEED_AMPLIFICATION_FACTOR 100
#define TWO_THIRDS 0.6667
#define ONE_HALF 0.5

const double HIPPO_MASS = 50;
const rgb_color_t HIPPO_COLOR = {0.0, 1.0, 1.0};
const double HIPPO_SIDE_SPEED = 0.010;
const double HIPPO_FORWARD_BACK_SPEED = 5 * SPEED_AMPLIFICATION_FACTOR;
const double RESCALE_IN_TO_EAT_FACTOR = 1.0 / 3.0;

const double BALL_RESOLUTION = 20;
const double BALL_APPROX_RADIUS = 33;
const rgb_color_t BALL_COLOR = {0.2, 0.2, 0.2};
const rgb_color_t CENTER_BALL_COLOR = {1.0, 0.0, 0.0};
const double BALL_SPEED = 3 * SPEED_AMPLIFICATION_FACTOR;
// note that weights should add up to MAX_BALLS_THIS_ROUND for weighting to work
// properly it is very okay to change the number of max-balls-this-round via the
// num-each-type parameter, and the relative weighting of each ball type not
// okay to mess with the order of these! if you need to eliminate one, either
// delete it from all the constants below (gross) or just make it equal to 0
// (much better idea)
#define NUM_EACH_TYPE 1
#define WEIGHT_DEFAULT_BALL_DU 5
#define WEIGHT_EAT_KILL_BALL_DU 0
#define WEIGHT_EAT_FAST_BALL_DU 1
#define WEIGHT_EAT_SLOW_BALL_DU 1
#define WEIGHT_EAT_SPILL_BALL_DU 0
#define WEIGHT_PLAYER_FAST_BALL_DU 1
#define WEIGHT_PLAYER_MORE_ANGLE_BALL_DU 1
#define WEIGHT_SHOOT_KILL_DU 0
#define WEIGHT_SHOOT_SLOW_DU 1
#define WEIGHT_SHOOT_SPILL_DU 0
// #define WEIGHT_WORLD_FULL_ANGLE_BALL_DU 0
// #define WEIGHT_WORLD_MORE_ANGLE_BALL_DU 0
// #define WEIGHT_WORLD_RESET_ARENA_BALL_DU 0
// #define WEIGHT_WORLD_FAST_BALL 0
const int WEIGHT_DEFAULT_BALL = WEIGHT_DEFAULT_BALL_DU;
const int WEIGHT_EAT_KILL_BALL
    = WEIGHT_DEFAULT_BALL_DU + WEIGHT_EAT_KILL_BALL_DU;
const int WEIGHT_EAT_FAST_BALL = WEIGHT_DEFAULT_BALL_DU
                                 + WEIGHT_EAT_KILL_BALL_DU
                                 + WEIGHT_EAT_FAST_BALL_DU;
const int WEIGHT_EAT_SLOW_BALL
    = WEIGHT_DEFAULT_BALL_DU + WEIGHT_EAT_KILL_BALL_DU + WEIGHT_EAT_FAST_BALL_DU
      + WEIGHT_EAT_SLOW_BALL_DU;
const int WEIGHT_EAT_SPILL_BALL
    = WEIGHT_DEFAULT_BALL_DU + WEIGHT_EAT_KILL_BALL_DU + WEIGHT_EAT_FAST_BALL_DU
      + WEIGHT_EAT_SLOW_BALL_DU + WEIGHT_EAT_SPILL_BALL_DU;
const int WEIGHT_PLAYER_FAST_BALL
    = WEIGHT_DEFAULT_BALL_DU + WEIGHT_EAT_KILL_BALL_DU + WEIGHT_EAT_FAST_BALL_DU
      + WEIGHT_EAT_SLOW_BALL_DU + WEIGHT_EAT_SPILL_BALL_DU
      + WEIGHT_PLAYER_FAST_BALL_DU;
const int WEIGHT_PLAYER_MORE_ANGLE_BALL
    = WEIGHT_DEFAULT_BALL_DU + WEIGHT_EAT_KILL_BALL_DU + WEIGHT_EAT_FAST_BALL_DU
      + WEIGHT_EAT_SLOW_BALL_DU + WEIGHT_EAT_SPILL_BALL_DU
      + WEIGHT_PLAYER_FAST_BALL_DU + WEIGHT_PLAYER_MORE_ANGLE_BALL_DU;
const int WEIGHT_SHOOT_KILL
    = WEIGHT_DEFAULT_BALL_DU + WEIGHT_EAT_KILL_BALL_DU + WEIGHT_EAT_FAST_BALL_DU
      + WEIGHT_EAT_SLOW_BALL_DU + WEIGHT_EAT_SPILL_BALL_DU
      + WEIGHT_PLAYER_FAST_BALL_DU + WEIGHT_PLAYER_MORE_ANGLE_BALL_DU
      + WEIGHT_SHOOT_KILL_DU;
const int WEIGHT_SHOOT_SLOW
    = WEIGHT_DEFAULT_BALL_DU + WEIGHT_EAT_KILL_BALL_DU + WEIGHT_EAT_FAST_BALL_DU
      + WEIGHT_EAT_SLOW_BALL_DU + WEIGHT_EAT_SPILL_BALL_DU
      + WEIGHT_PLAYER_FAST_BALL_DU + WEIGHT_PLAYER_MORE_ANGLE_BALL_DU
      + WEIGHT_SHOOT_KILL_DU + WEIGHT_SHOOT_SLOW_DU;
const int WEIGHT_SHOOT_SPILL
    = WEIGHT_DEFAULT_BALL_DU + WEIGHT_EAT_KILL_BALL_DU + WEIGHT_EAT_FAST_BALL_DU
      + WEIGHT_EAT_SLOW_BALL_DU + WEIGHT_EAT_SPILL_BALL_DU
      + WEIGHT_PLAYER_FAST_BALL_DU + WEIGHT_PLAYER_MORE_ANGLE_BALL_DU
      + WEIGHT_SHOOT_KILL_DU + WEIGHT_SHOOT_SLOW_DU + WEIGHT_SHOOT_SPILL_DU;
const int MAX_BALLS_THIS_ROUND
    = (WEIGHT_DEFAULT_BALL_DU + WEIGHT_EAT_KILL_BALL_DU
       + WEIGHT_EAT_FAST_BALL_DU + WEIGHT_EAT_SLOW_BALL_DU
       + WEIGHT_EAT_SPILL_BALL_DU + WEIGHT_PLAYER_FAST_BALL_DU
       + WEIGHT_PLAYER_MORE_ANGLE_BALL_DU + WEIGHT_SHOOT_KILL_DU
       + WEIGHT_SHOOT_SLOW_DU + WEIGHT_SHOOT_SPILL_DU)
      * NUM_EACH_TYPE;

const double WALL_MASS = INFINITY;
const double WALL_THICKNESS = 50;
const rgb_color_t WALL_COLOR = {0.5, 0.5, 0.5};
const rgb_color_t ARENA_COLOR = {0.0, 0.8, 0.0};
const rgb_color_t OCEAN_COLOR = {0.0, 0.1, 0.5};
const double WALL_RESOLUTION = 100;
const char *BACKGROUND_PATH_SHAPE = "static/background.csv";

const double ELASTICITY = 1.0;
const double FRICTION = 0.5;
const double SPRINKLER_MOD_FACTOR = 25;
const double MAX_SPEED = 5.0; // ?? to be decided later

const vector_t PLAYER_TAB_POSITION = {30, 800};
const double MIN_TIME_FOR_ROUND = 5.0;

// make the info type for body_init_with_info
typedef enum info { BALL_INFO, WALL_INFO } info_e;

// TODO: add prototypes and comments
vector_t hh_hippo_positions(double num_hippos, double curr_hippo);
body_t *hh_init_ball(vector_t initial_position,
                     void *curr_info,
                     rgb_color_t color,
                     double resolution);
body_t *
hh_init_hippo(vector_t initial_position, void *curr_info, rgb_color_t color);
body_t *hh_init_background_circle();
body_t *hh_init_arena(double thickness,
                      double radius,
                      double resolution,
                      void *arena_info,
                      free_func_t freer,
                      rgb_color_t color);
game_world_t *hh_world_init(scene_t *hh_world, double num_hippos);
void hh_on_key(key_index_e key, key_event_type_t type, hh_key_aux_t *aux);
void hh_abstracted_spawn_balls(game_world_t *hh_game,
                               int *total_balls_created,
                               vector_t initial_velocity);
void hh_spawn_balls_sprinkler(game_world_t *hh_game,
                              double total_time,
                              int *total_balls_created);
bool hh_rounds(game_world_t *world,
               int *total_balls_created,
               int *curr_round,
               bool *win_condition_reached,
               hh_key_aux_t *key_aux);
bool hh_win_condition(game_world_t *world,
                      int *total_balls_created,
                      int *curr_round,
                      hh_key_aux_t *key_aux);
// the list of indices to make refers to what sprite should be used to
// initialize the players
void hh_make_hippos(game_world_t *game,
                    double num_hippos,
                    list_t *indices_to_make);
Uint32 hh_move_back_callback(Uint32 interval, body_t *body);
Uint32 hh_allow_eat_callback(Uint32 interval, player_t *player);

// TODO: fix prototypes

// Function Definitions

// num_hippos is 1 indexed, curr_hippo is 1 indexed
vector_t hh_hippo_positions(double num_hippos, double curr_hippo) {
    if ((num_hippos < 1) || (num_hippos > 5)) {
        num_hippos = 1;
    }
    if (curr_hippo > num_hippos) {
        curr_hippo = 1;
    }
    double angle_rotate = 2 * M_PI / num_hippos * (curr_hippo - 1);
    vector_t subtract_by = {.x = 0.0, .y = WORLD_RADIUS};
    vector_t default_vector = vec_subtract(CENTER_WORLD, subtract_by);
    return vec_rotate_relative(default_vector, angle_rotate, CENTER_WORLD);
}

body_t *hh_init_background_circle() {
    return make_circle(WORLD_RADIUS,
                       WALL_RESOLUTION,
                       CENTER_WORLD,
                       INFINITY,
                       NULL,
                       NULL,
                       WALL_COLOR);
}

body_t *hh_init_arena(double thickness,
                      double radius,
                      double resolution,
                      void *arena_info,
                      free_func_t freer,
                      rgb_color_t color) {
    body_t *arena_outer = make_circle(radius + thickness,
                                      resolution,
                                      CENTER_WORLD,
                                      INFINITY,
                                      arena_info,
                                      freer,
                                      color);
    return arena_outer;
}

void hh_make_hippos(game_world_t *game,
                    double num_hippos,
                    list_t *indices_to_make) {
    for (int i = 0; i < num_hippos; i++) {
        player_t *new_hippo = player_init(*(int *)list_get(indices_to_make, i));
        body_t *new_hippo_body = player_get_body(new_hippo);
        body_set_centroid(new_hippo_body,
                          hh_hippo_positions(num_hippos, i + 1));
        double angle_rotate = 2 * M_PI / num_hippos * (i);
        body_set_rotation(new_hippo_body, angle_rotate);
        game_world_add_hippo(game, new_hippo);
        player_set_state(new_hippo, PLAYER_CHILLING);
    }
}

game_world_t *hh_world_init(scene_t *hh_world, double num_hippos) {
    game_world_t *game = game_world_init(hh_world,
                                         ELASTICITY,
                                         MAX_ANGLE,
                                         MAX_SPEED,
                                         num_hippos - 1,
                                         CENTER_WORLD);
    // make the background
    sprite_t *sprite
        = sprite_init("static/CS_3_background.png", 1.0 / 2.0, VEC_ZERO);
    list_t *sprites = list_init(1, (free_func_t)sprite_free);
    list_add(sprites, sprite);
    list_t *shapes = list_init(1, (free_func_t)list_free);
    list_t *background = polygon_init_from_path(BACKGROUND_PATH_SHAPE);
    polygon_scale(background, 1.0);
    list_add(shapes, background);
    gfx_aux_t *gfx = gfx_aux_init(sprites);
    body_t *background_body
        = body_init_with_gfx(INFINITY, NULL, NULL, shapes, gfx);
    body_set_centroid(background_body, CENTER_WORLD);
    game_world_set_background(game, background_body);

    // add hippos to the world
    list_t *indices_to_make = list_init(5, free);
    int *curr_int;
    for (int i = 0; i < num_hippos; i++) {
        curr_int = (int *)malloc(sizeof(int));
        *curr_int = i;
        list_add(indices_to_make, curr_int);
    }
    hh_make_hippos(game, num_hippos, indices_to_make);
    list_free(indices_to_make);

    return game;
}

void hh_on_key(key_index_e key, key_event_type_t type, hh_key_aux_t *aux) {
    game_world_t *world = aux->game;
    double angle_modifier = game_world_get_angle_modifier(world, 0);
    player_t *hippo0_player = kh_get_hippo_from_array(aux, 0);
    player_t *hippo1_player = kh_get_hippo_from_array(aux, 1);
    player_t *hippo2_player = kh_get_hippo_from_array(aux, 2);
    player_t *hippo3_player = kh_get_hippo_from_array(aux, 3);
    player_t *hippo4_player = kh_get_hippo_from_array(aux, 4);
    body_t *hippo0_body = NULL;
    body_t *hippo1_body = NULL;
    body_t *hippo2_body = NULL;
    body_t *hippo3_body = NULL;
    body_t *hippo4_body = NULL;
    if (hippo0_player != NULL) {
        hippo0_body = player_get_body(hippo0_player);
    }
    if (hippo1_player != NULL) {
        hippo1_body = player_get_body(hippo1_player);
    }
    if (hippo2_player != NULL) {
        hippo2_body = player_get_body(hippo2_player);
    }
    if (hippo3_player != NULL) {
        hippo3_body = player_get_body(hippo3_player);
    }
    if (hippo4_player != NULL) {
        hippo4_body = player_get_body(hippo4_player);
    }

    // Left and Right are mutually exclusive
    if ((key == KEY_LEFT) || (key == KEY_A) || (key == KEY_H)
        || (key == KEY_1)) {
        // move left
        player_t *curr_player = NULL;
        body_t *curr_body = NULL;
        int curr_hippo = -1;
        if (key == KEY_LEFT) {
            curr_player = hippo0_player;
            curr_body = hippo0_body;
            curr_hippo = 1;
        } else if (key == KEY_A) {
            curr_player = hippo1_player;
            curr_body = hippo1_body;
            curr_hippo = 2;
        } else if (key == KEY_H) {
            curr_player = hippo2_player;
            curr_body = hippo2_body;
            curr_hippo = 3;
        } else if (key == KEY_1) {
            curr_player = hippo3_player;
            curr_body = hippo3_body;
            curr_hippo = 4;
        }
        if (curr_body != NULL) {
            hippo_movement_side(
                curr_body,
                CENTER_WORLD,
                hh_hippo_positions(game_world_get_num_hippos(world),
                                   curr_hippo),
                WORLD_RADIUS,
                MAX_ANGLE + angle_modifier,
                player_get_speed_factor(curr_player) * HIPPO_SIDE_SPEED,
                -1.0);
            // key-released is unnecessary because side to side movement deals
            // with translocation rather than velocity
        }
    } else if ((key == KEY_RIGHT) || (key == KEY_D) || (key == KEY_L)
               || (key == KEY_3)) {
        // move right
        player_t *curr_player = NULL;
        body_t *curr_body = NULL;
        int curr_hippo = -1;
        if (key == KEY_RIGHT) {
            curr_player = hippo0_player;
            curr_body = hippo0_body;
            curr_hippo = 1;
        } else if (key == KEY_D) {
            curr_player = hippo1_player;
            curr_body = hippo1_body;
            curr_hippo = 2;
        } else if (key == KEY_L) {
            curr_player = hippo2_player;
            curr_body = hippo2_body;
            curr_hippo = 3;
        } else if (key == KEY_3) {
            curr_player = hippo3_player;
            curr_body = hippo3_body;
            curr_hippo = 4;
        }
        if (curr_player != NULL) {
            hippo_movement_side(
                curr_body,
                CENTER_WORLD,
                hh_hippo_positions(game_world_get_num_hippos(world),
                                   curr_hippo),
                WORLD_RADIUS,
                MAX_ANGLE + angle_modifier,
                player_get_speed_factor(curr_player) * HIPPO_SIDE_SPEED,
                1.0);
            // key-released is unnecessary because side to side movement deals
            // with translocation rather than velocity
        }
    }
    if ((key == KEY_UP) || (key == KEY_W) || (key == KEY_K) || (key == KEY_5)) {
        // move forward
        player_t *curr_player = NULL;
        body_t *curr_body;
        int curr_hippo = -1;
        if (key == KEY_UP) {
            curr_player = hippo0_player;
            curr_body = hippo0_body;
            curr_hippo = 0;
        } else if (key == KEY_W) {
            curr_player = hippo1_player;
            curr_body = hippo1_body;
            curr_hippo = 1;
        } else if (key == KEY_K) {
            curr_player = hippo2_player;
            curr_body = hippo2_body;
            curr_hippo = 2;
        } else if (key == KEY_5) {
            curr_player = hippo3_player;
            curr_body = hippo3_body;
            curr_hippo = 3;
        }
        if (!player_is_eating(curr_player)) {
            // don't allow the player to move farther forward if they've already
            // moved in
            hippo_movement_forward(curr_body,
                                   CENTER_WORLD,
                                   RESCALE_IN_TO_EAT_FACTOR);
            SDL_AddTimer(HIPPO_EAT_TIME,
                         (SDL_TimerCallback)hh_move_back_callback,
                         curr_body);
            SDL_AddTimer(HIPPO_EAT_WAIT_TIME,
                         (SDL_TimerCallback)hh_allow_eat_callback,
                         curr_player);
            player_set_state(curr_player, PLAYER_EATING);
        }
    }
    if ((key == KEY_DOWN) || (key == KEY_S) || (key == KEY_J)
        || (key == KEY_2)) {
        // shoot
        player_t *curr_player = NULL;
        body_t *curr_body;
        int curr_hippo = -1;
        if (key == KEY_DOWN) {
            curr_player = hippo0_player;
            curr_body = hippo0_body;
            curr_hippo = 0;
        } else if (key == KEY_S) {
            curr_player = hippo1_player;
            curr_body = hippo1_body;
            curr_hippo = 1;
        } else if (key == KEY_J) {
            curr_player = hippo2_player;
            curr_body = hippo2_body;
            curr_hippo = 2;
        } else if (key == KEY_2) {
            curr_player = hippo3_player;
            curr_body = hippo3_body;
            curr_hippo = 3;
        }
        if (curr_player != NULL) {
            if (kh_currently_pressed(key, aux)) {
                if (!kh_currently_shot(aux, curr_hippo)) {
                    printf("shot\n");
                    player_activate_powerup(curr_player, world);
                    aux->already_shot_this_press_array[curr_hippo] = true;
                }
            } else { // you've released the key
                if (kh_currently_shot(aux, curr_hippo)) {
                    printf("released\n");
                    aux->already_shot_this_press_array[curr_hippo] = false;
                }
            }
        }
    }
    if ((key == KEY_SLASH) || (key == KEY_E) || (key == KEY_I)
        || (key == KEY_4)) {
        // shoot
        player_t *curr_player = NULL;
        body_t *curr_body;
        int curr_hippo = -1;
        if (key == KEY_SLASH) {
            curr_player = hippo0_player;
            curr_body = hippo0_body;
            curr_hippo = 0;
        } else if (key == KEY_E) {
            curr_player = hippo1_player;
            curr_body = hippo1_body;
            curr_hippo = 1;
        } else if (key == KEY_I) {
            curr_player = hippo2_player;
            curr_body = hippo2_body;
            curr_hippo = 2;
        } else if (key == KEY_4) {
            curr_player = hippo3_player;
            curr_body = hippo3_body;
            curr_hippo = 3;
        }
        if (curr_player != NULL) {
            if (kh_currently_pressed(key, aux)) {
                if (!kh_currently_switched(aux, curr_hippo)) {
                    printf("switched\n");
                    player_inc_powerup_idx(curr_player, 1);
                    aux->already_switched_this_press_array[curr_hippo] = true;
                }
            } else { // you've released the key
                if (kh_currently_switched(aux, curr_hippo)) {
                    printf("un-switched\n");
                    aux->already_switched_this_press_array[curr_hippo] = false;
                }
            }
        }
    }
}

ball_power_type_e hh_abstracted_ball_type_weighting() {
    int new_ball_type = rand() / (double)RAND_MAX * MAX_BALLS_THIS_ROUND;
    ball_power_type_e type_new_ball;
    if (new_ball_type <= WEIGHT_DEFAULT_BALL) {
        type_new_ball = POWER_NONE;
    } else if (new_ball_type <= WEIGHT_EAT_KILL_BALL) {
        type_new_ball = POWER_PLAYER_EAT_KILL;
    } else if (new_ball_type <= WEIGHT_EAT_FAST_BALL) {
        type_new_ball = POWER_PLAYER_EAT_FAST;
    } else if (new_ball_type <= WEIGHT_EAT_SLOW_BALL) {
        type_new_ball = POWER_PLAYER_EAT_SLOW;
    } else if (new_ball_type <= WEIGHT_EAT_SPILL_BALL) {
        type_new_ball = POWER_PLAYER_EAT_SPILL;
    } else if (new_ball_type <= WEIGHT_PLAYER_FAST_BALL) {
        type_new_ball = POWER_PLAYER_ACTIVATE_FAST;
    } else if (new_ball_type <= WEIGHT_PLAYER_MORE_ANGLE_BALL) {
        type_new_ball = POWER_PLAYER_ACTIVATE_MORE_ANGLE;
    } else if (new_ball_type <= WEIGHT_SHOOT_KILL) {
        type_new_ball = POWER_PLAYER_SHOOT_KILL;
    } else if (new_ball_type <= WEIGHT_SHOOT_SLOW) {
        type_new_ball = POWER_PLAYER_SHOOT_SLOW;
    } else if (new_ball_type <= WEIGHT_SHOOT_SPILL) {
        type_new_ball = POWER_PLAYER_SHOOT_SPILL;
    } else { // should not reach this condition, but just in case, make it
             // default ball
        type_new_ball = POWER_NONE;
    }
    return type_new_ball;
}

void hh_abstracted_spawn_balls(game_world_t *hh_game,
                               int *total_balls_created,
                               vector_t initial_velocity) {
    if (*total_balls_created > MAX_BALLS_THIS_ROUND) {
        return;
    }
    ball_power_type_e type_new_ball = hh_abstracted_ball_type_weighting();
    spawn_ball(hh_game, CENTER_WORLD, initial_velocity, type_new_ball);
    *total_balls_created += 1;
}
void hh_spawn_balls_sprinkler(game_world_t *hh_game,
                              double total_time,
                              int *total_balls_created) {
    double angle_initial_velocity = fmod(total_time, SPRINKLER_MOD_FACTOR);
    vector_t initial_velocity
        = vec_rotate(vec_multiply(BALL_SPEED, E1), angle_initial_velocity);
    hh_abstracted_spawn_balls(hh_game, total_balls_created, initial_velocity);
}

void hh_spawn_balls_random_angle(game_world_t *hh_game,
                                 int *total_balls_created) {
    double angle_initial_velocity = (rand() / (double)RAND_MAX * M_PI * 2);
    vector_t initial_velocity
        = vec_rotate(vec_multiply(BALL_SPEED, E1), angle_initial_velocity);
    hh_abstracted_spawn_balls(hh_game, total_balls_created, initial_velocity);
}

// Note: this should only be called once per round, unlike the other
// spawn-balls-methods that are called on regular time intervals
// total_balls_to_make will be approximated to the nearest square
void hh_spawn_balls_pool(game_world_t *hh_game, int total_balls_to_make) {
    int balls_per_row = (int)sqrt(total_balls_to_make);
    double width_row = (balls_per_row * 2 * BALL_APPROX_RADIUS);
    vector_t upper_left
        = vec_subtract(CENTER_WORLD,
                       (vector_t){.x = width_row / 2 - BALL_APPROX_RADIUS,
                                  .y = width_row / 2 - BALL_APPROX_RADIUS});
    double single_ball = 2 * BALL_APPROX_RADIUS;
    for (int row = 0; row < balls_per_row; row++) {
        for (int col = 0; col < balls_per_row; col++) {
            vector_t initial_position = vec_add(
                vec_add(upper_left, vec_multiply((single_ball * row), E1)),
                vec_multiply((single_ball * col), E2));
            ball_power_type_e type_new_ball
                = hh_abstracted_ball_type_weighting();
            spawn_ball(hh_game, initial_position, VEC_ZERO, type_new_ball);
        }
    }
}

Uint32 hh_move_back_callback(Uint32 interval, body_t *body) {
    hippo_movement_backward(body,
                            CENTER_WORLD,
                            RESCALE_IN_TO_EAT_FACTOR,
                            WORLD_RADIUS);
    return 0;
}

Uint32 hh_allow_eat_callback(Uint32 interval, player_t *player) {
    player_set_state(player, PLAYER_CHILLING);
    return 0;
}

void hh_check_collisions(game_world_t *hh_game) {
    list_t *balls = game_world_get_balls(hh_game);
    for (int i = 0; i < list_size(balls); i++) {
        body_t *curr_ball = list_get(balls, i);
        boundary_arena_ball_collision(curr_ball,
                                      CENTER_WORLD,
                                      WORLD_RADIUS - BALL_APPROX_RADIUS,
                                      BALL_APPROX_RADIUS,
                                      BUFFER_ZONE,
                                      BALL_SPEED);
        reset_boundary_balls(curr_ball,
                             CENTER_WORLD,
                             WORLD_RADIUS,
                             BUFFER_ZONE,
                             BALL_SPEED);
    }
}

bool hh_win_condition(game_world_t *world,
                      int *total_balls_created,
                      int *curr_round,
                      hh_key_aux_t *key_aux) {
    // if all the balls are gone, the game is over
    if (game_world_get_num_balls(world) <= 0) {
        list_t *list_hippos = game_world_get_hippos(world);
        short least_points = player_get_points(list_get(list_hippos, 0));
        player_t *player_to_remove = list_get(list_hippos, 0);

        for (int i = 0; i < list_size(list_hippos); i++) {
            player_t *current_hippo = list_get(list_hippos, i);
            if (player_get_points(current_hippo) < least_points) {
                least_points = player_get_points(current_hippo);
                player_to_remove = current_hippo;
            }
        }

        // Remove the hippo with the least amount of points at the end of each
        // round.
        game_world_remove_hippo(world, player_to_remove);
        printf("Player %zu had the lowest number of points in this round and "
               "was eliminated\n",
               player_get_idx(player_to_remove));
        // note: all bodies marked for removal, not yet deleted so pointers
        // still work, but removed from lists in game_world

        // This delay should be here so that the last hh_move_back_callback is
        // called before the hippos are freed.
        SDL_Delay(HIPPO_EAT_TIME * 2);
        game_world_reset_arena(world, ELASTICITY, MAX_ANGLE, MAX_SPEED);
        // add hippos to the world
        int *curr_int;
        // curr round is 1 less than the number of hippos in the last round
        int new_num_hippos = *curr_round;
        list_t *indices_to_make = list_init(new_num_hippos, free);
        // NOTE: skips index of player deleted last round, hence + 1
        for (int i = 0; i < new_num_hippos + 1; i++) {
            if (i != player_get_idx(player_to_remove)) {
                curr_int = (int *)malloc(sizeof(int));
                *curr_int = i;
                list_add(indices_to_make, curr_int);
            }
        }
        hh_make_hippos(world, new_num_hippos, indices_to_make);
        printf("made new hippos succesfully in rounds \n");

        hh_key_aux_reset(world, key_aux);

        if (list_size(game_world_get_hippos(world)) <= 1) {
            printf("Game over! Player %zd won! \n",
                   player_get_idx(game_world_get_hippo(world, 0)));
            return SPECIAL_CASE_GAME_OVER;
        }

        return true;
    }
    return false;
}

// returns true if reseting the world
bool hh_rounds(game_world_t *world,
               int *total_balls_created,
               int *curr_round,
               bool *win_condition_reached,
               hh_key_aux_t *key_aux) {
    if (*win_condition_reached) {
        return false;
    } else {
        if (hh_win_condition(world, total_balls_created, curr_round, key_aux)) {
            *win_condition_reached = true;
            // the current round is the initial number of hippos in the previous
            // round - 1
            *curr_round -= 1;
            printf("reset and decrementing round. new round num is: %d \n",
                   *curr_round);

            game_world_decrement_round(world);
            return true;
        } else {
            return false;
        }
    }
}

int main(int argc, char **argv) {
    graphics_t *graphics = graphics_init(MAX);
    scene_t *hh_world = scene_init();
    game_world_t *hh_game = hh_world_init(hh_world, INIT_NUM_HIPPOS);

    hh_key_aux_t *key_aux
        = hh_key_aux_init(hh_game, (key_handler_enumed_t)hh_on_key);
    // updates the list of booleans every key_event that happens
    sdl_on_key((key_handler_t)kh_update_bool_list_char, key_aux);

    srand(time(NULL));

    double dt = 0.0;
    double spawn_ball_time = 0.0;
    double total_time = 0.0;
    int *total_balls_created = malloc(sizeof(int));
    *total_balls_created = 0;
    int *curr_round = malloc(sizeof(int));
    *curr_round = INIT_NUM_HIPPOS - 1;

    if (SPAWN_POOL_METHOD) {
        hh_spawn_balls_pool(hh_game,
                            MAX_BALLS_THIS_ROUND < MAX_BALLS_THAT_FIT_ON_SCREEN
                                ? MAX_BALLS_THIS_ROUND
                                : MAX_BALLS_THAT_FIT_ON_SCREEN);
    }

    list_t *tabs = list_init(1, (free_func_t)text_tab_free);
    list_add(
        tabs,
        player_create_tab(game_world_get_hippos(hh_game), PLAYER_TAB_POSITION));
    graphics_add_text_tabs(graphics, tabs);
    graphics_add_bodies(graphics, scene_get_bodies(hh_world));

    bool *win_condition_reached = malloc(sizeof(bool));
    *win_condition_reached = false;

    while (!sdl_is_done()) {
        dt = time_since_last_tick();
        total_time += dt;
        if (total_time > WAIT_SPAWN_BALLS_TIME) {
            if (SPAWN_SPRINKLER_METHOD || SPAWN_RANDOM_ANGLE_METHOD) {
                spawn_ball_time += dt;
                if (spawn_ball_time > CONTINOUS_SPAWN_TIME_INTERVAL) {
                    if (SPAWN_SPRINKLER_METHOD) {
                        hh_spawn_balls_sprinkler(hh_game,
                                                 total_time,
                                                 total_balls_created);
                    } else if (SPAWN_RANDOM_ANGLE_METHOD) {
                        hh_spawn_balls_random_angle(hh_game,
                                                    total_balls_created);
                    }
                    spawn_ball_time = 0.0;
                }
            }
        }
        // functions that need to be called every tick
        hh_check_collisions(hh_game);
        kh_helping_on_key(key_aux);
        if (total_time > MIN_TIME_FOR_ROUND + WAIT_SPAWN_BALLS_TIME) {
            bool game_status = hh_rounds(hh_game,
                                         total_balls_created,
                                         curr_round,
                                         win_condition_reached,
                                         key_aux);
            // if game_status == SPECIAL_CASE_GAME_OVER or curr_round <= 0
            // do not change total time, total_balls_created or
            // win_condition_reached need the game to stall out (i.e. stop
            // decrememnting rounds and spawning new balls without crashing
            if ((game_status == 1) && (*curr_round >= 1)) {
                total_time = -WAIT_RESTART_ROUND_TIME;
                printf("undergoing round wait time, please wait for the next "
                       "round to start \n");
                *total_balls_created = 0;
                *win_condition_reached = false;
            }
        }
        game_world_tick(hh_game, dt);
        graphics_render(graphics);
    }

    free(curr_round);
    free(win_condition_reached);
    free(total_balls_created);
    hh_key_aux_free(key_aux);
    scene_free(hh_world);
    game_world_free(hh_game);
    list_free(tabs);
    graphics_free(graphics);

    return 0;
}
