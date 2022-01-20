#include "game_world.h"
#include "ball.h"
#include "body.h"
#include "list.h"
#include "player.h"
#include "scene.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// STRUCTS

typedef struct game_world {
    list_t *hippos;
    double num_hippos;
    list_t *balls;
    double num_balls;
    double elasticity;
    double max_angle;
    double *angle_modifiers;
    double max_speed;
    double num_rounds;
    vector_t arena_center;
    body_t *background;
    scene_t *scene;
} game_world_t;

game_world_t *game_world_init(scene_t *scene,
                              double elasticity,
                              double max_angle,
                              double max_speed,
                              double num_rounds,
                              vector_t arena_center) {
    game_world_t *curr_game = malloc(sizeof(game_world_t));
    curr_game->hippos = list_init(1, NULL);
    curr_game->num_hippos = 0;
    // Ball are bodies that are freed from scene.
    curr_game->balls = list_init(1, NULL);
    curr_game->num_balls = 0;
    curr_game->elasticity = elasticity;
    curr_game->max_angle = max_angle;
    curr_game->max_speed = max_speed;
    curr_game->num_rounds = num_rounds;
    curr_game->background = NULL;
    curr_game->scene = scene;
    curr_game->arena_center = arena_center;
    curr_game->angle_modifiers = calloc(4, sizeof(double));
    return curr_game;
}

// PRIVATE FUNCTIONS

/**
 * Checks if 'idx' is within the range for the current number of players.
 * Return true if so.
 */
bool game_world_check_player_idx(game_world_t *world, size_t idx);

// GETTERS

scene_t *game_world_get_scene(game_world_t *world) {
    return world->scene;
}

list_t *game_world_get_hippos(game_world_t *world) {
    return world->hippos;
}

player_t *game_world_get_hippo(game_world_t *world, size_t idx) {
    return list_get(world->hippos, idx);
}

double game_world_get_num_hippos(game_world_t *world) {
    return world->num_hippos;
}

list_t *game_world_get_balls(game_world_t *world) {
    return world->balls;
}

double game_world_get_num_balls(game_world_t *world) {
    return world->num_balls;
}

double game_world_get_elasticity(game_world_t *world) {
    return world->elasticity;
}

double game_world_get_max_angle(game_world_t *world) {
    return world->max_angle;
}

double game_world_get_max_speed(game_world_t *world) {
    return world->max_speed;
}

double game_world_get_num_rounds(game_world_t *world) {
    return world->num_rounds;
}

vector_t game_world_get_arena_center(game_world_t *world) {
    return world->arena_center;
}

double game_world_get_angle_modifier(game_world_t *world, size_t idx) {
    if (game_world_check_player_idx(world, idx)) {
        return player_get_angle_modifier(game_world_get_hippo(world, idx));
    }
    return 0;
}

body_t *game_world_get_background(game_world_t *world) {
    return world->background;
}

// SETTERS

// TODO fix to using just add and remove so it updates scene too
void game_world_set_hippos(game_world_t *world, list_t *new_list_hippos) {
    world->hippos = new_list_hippos;
    world->num_hippos = list_size(new_list_hippos);
}
// TODO same
void game_world_set_balls(game_world_t *world, list_t *new_list_balls) {
    world->balls = new_list_balls;
    world->num_balls = list_size(new_list_balls);
}

void game_world_set_elasticity(game_world_t *world, double elasticity) {
    world->elasticity = elasticity;
}

void game_world_set_max_angle(game_world_t *world, double max_angle) {
    world->max_angle = max_angle;
}

void game_world_set_max_speed(game_world_t *world, double max_speed) {
    world->max_speed = max_speed;
}

void game_world_set_num_rounds(game_world_t *world, double num_rounds) {
    world->num_rounds = num_rounds;
}

void game_world_set_arena_center(game_world_t *world, vector_t arena_center) {
    world->arena_center = arena_center;
}

void game_world_set_scene(game_world_t *world, scene_t *scene) {
    world->scene = scene;
}

void game_world_set_background(game_world_t *world, body_t *background) {
    // TODO: is this a good idea?
    scene_add_body(world->scene, background);
    world->background = background;
}

// FUNCTIONS

void game_world_add_hippo(game_world_t *world, player_t *hippo) {
    list_add(world->hippos, hippo);
    scene_add_body(world->scene, player_get_body(hippo));
    world->num_hippos += 1;
}

void game_world_add_ball(game_world_t *world, body_t *ball) {
    list_add(world->balls, ball);
    scene_add_body(world->scene, ball);
    world->num_balls += 1;
}

player_t *game_world_remove_hippo(game_world_t *world,
                                  player_t *hippo_to_be_removed) {
    // find the hippo and remove it from the game world
    for (int j = world->num_hippos - 1; j >= 0; j--) {
        player_t *curr_hippo = list_get(world->hippos, j);
        if (hippo_to_be_removed == curr_hippo) {
            list_remove(world->hippos, j);
            break;
        }
    }
    // player_remove marks player and its body for removal
    player_remove(hippo_to_be_removed);
    world->num_hippos -= 1;
    return hippo_to_be_removed;
}

body_t *game_world_remove_ball(game_world_t *world,
                               body_t *ball_to_be_removed) {
    // find the ball and remove it from the game_world
    for (int j = world->num_balls - 1; j >= 0; j--) {
        body_t *curr_ball = list_get(world->balls, j);
        if (ball_to_be_removed == curr_ball) {
            list_remove(world->balls, j);
            break;
        }
    }
    body_remove(ball_to_be_removed);
    world->num_balls -= 1;
    return ball_to_be_removed;
}

double game_world_decrement_round(game_world_t *world) {
    double curr_round = world->num_rounds;
    world->num_rounds -= 1;
    return curr_round;
}

void game_world_increase_angle_modifier(game_world_t *world,
                                        size_t idx,
                                        double range) {
    if (game_world_check_player_idx(world, idx)) {
        player_add_angle_modifier(game_world_get_hippo(world, idx), range);
    }
}

void game_world_set_angle_modifier(game_world_t *world,
                                   size_t idx,
                                   double range) {
    if (game_world_check_player_idx(world, idx)) {
        player_set_angle_modifier(game_world_get_hippo(world, idx), range);
    }
}

void game_world_reset_arena(game_world_t *world,
                            double elasticity,
                            double max_angle,
                            double max_speed) {
    // TODO
    // reset all speeds to default
    list_t *balls = world->balls;
    for (int i = list_size(balls) - 1; i >= 0; i--) {
        game_world_remove_ball(world, list_get(balls, i));
    }
    list_t *players = world->hippos;
    for (int j = list_size(players) - 1; j >= 0; j--) {
        game_world_remove_hippo(world, list_get(players, j));
    }
    world->elasticity = elasticity;
    world->max_angle = max_angle;
    world->max_speed = max_speed;
    world->angle_modifiers = calloc(4, sizeof(double));
    printf("all good in game_world_reset_arena \n");
}

void game_world_free(game_world_t *world) {
    // note, does not free scene
    free(world->angle_modifiers);
    list_free(world->balls);
    list_free(world->hippos);
    free(world);
}

void game_world_tick(game_world_t *world, double dt) {
    scene_tick(world->scene, dt);
}

bool game_world_check_player_idx(game_world_t *world, size_t idx) {
    return (idx < world->num_hippos) && (idx >= 0);
}
