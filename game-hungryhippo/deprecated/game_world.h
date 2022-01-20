#ifndef __GAME_WORLD_H__
#define __GAME_WORLD_H__

#include "list.h"
#include "scene.h"

/*** DEPENDENCIES ***/
typedef struct player player_t;

// STRUCTS

/**
 * defines a struct for the game that contains essential global
 * values as well as the list of players and balls
 */
typedef struct game_world game_world_t;

/**
 * initiates a game_world with given parameters and returns it.
 * initializes list of players and bodies to 0
 * @param scene the scene in which the game is happening
 * @param elasticity the elasticity of every collision in the world
 * @param max_angle the maximum angle of side to side motion for every hippo in
 * the world
 * @param max_speed the maximum speed of side to side motion for hippos in the
 * world
 * @param num_rounds the number of rounds the game has left
 * @param arena_center the vector_t center of the arena
 * @return a game_world with all these elements
 */
game_world_t *game_world_init(scene_t *scene,
                              double elasticity,
                              double max_angle,
                              double max_speed,
                              double num_rounds,
                              vector_t arena_center);

// GETTERS

/**
 * game_world_get_scene returns the scene of the world
 * @param world the game_world from which you are returning this value
 */
scene_t *game_world_get_scene(game_world_t *world);

/**
 * game_world_get_hippos returns the list of hippos in the world
 * @param world the game_world from which you are returning this value
 */
list_t *game_world_get_hippos(game_world_t *world);

/**
 * Return the hippo at index 'idx' if it is a valid index.
 * @param world the game-world from which you are returning this value
 * @param idx the index of the hippo player to return
 */
player_t *game_world_get_hippo(game_world_t *world, size_t idx);

/**
 * game_world_get_num_hippos returns the number of hippos in the world
 * @param world the game_world from which you are returning this value
 */
double game_world_get_num_hippos(game_world_t *world);

/**
 * game_world_get_balls returns the list of balls in the world
 * @param world the game_world from which you are returning this value
 */
list_t *game_world_get_balls(game_world_t *world);

/**
 * game_world_get_num_balls returns the number of balls in the world
 * @param world the game_world from which you are returning this value
 */
double game_world_get_num_balls(game_world_t *world);

/**
 * game_world_get_elasticity returns the global elasticity in the world
 * @param world the game_world from which you are returning this value
 */
double game_world_get_elasticity(game_world_t *world);

/**
 * game_world_get_max_angle returns the maximum angle of side to side motion
 * for hippos in the world
 * @param world the game_world from which you are returning this value
 */
double game_world_get_max_angle(game_world_t *world);

/**
 * game_world_get_max_speed returns the maximum speed of events in the world
 * @param world the game_world from which you are returning this value
 */
double game_world_get_max_speed(game_world_t *world);

/**
 * game_world_get_num_rounds returns the number of rounds in the world
 * @param world the game_world from which you are returning this value
 */
double game_world_get_num_rounds(game_world_t *world);

/**
 * Returns the centroid coordinate of the circular arena.
 * @param world the game world from which you are returning this value
 */
vector_t game_world_get_arena_center(game_world_t *world);

/**
 * Returns the angle modifier for the player of index 'idx'.
 */
double game_world_get_angle_modifier(game_world_t *world, size_t idx);

/**
 * Returns the background body for the world
 */
body_t *game_world_get_background(game_world_t *world);

// SETTERS

// TODO FIX THESE
/**
 * game_world_set_hippos sets the list of hippos in the world to the given
 * parameter Also updates the number of hippos in the world.
 * @param world the game_world in which you are setting this value
 * @param new_list_hippos the new list of hippos (player_t)
 */
void game_world_set_hippos(game_world_t *world, list_t *new_list_hippos);

/**
 * game_world_set_balls sets the list of balls in the world to the given
 * parameter Also updates the number of balls in the world.
 * @param world the game_world in which you are setting this value
 * @param new_list_balls the new list of balls (player_t)
 */
void game_world_set_balls(game_world_t *world, list_t *new_list_balls);

/**
 * game_world_set_elasticity sets the global elasticity in the world to the
 * given parameter
 * @param world the game_world in which you are setting this value
 * @param elasticity the new elasticity (<= 1)
 */
void game_world_set_elasticity(game_world_t *world, double elasticity);

/**
 * game_world_set_max_angle sets the global maximum angle of side to side motion
 * for hippos in the world to the given parameter
 * @param world the game_world in which you are setting this value
 * @param max_angle the new maximum angle (<= 2 pi / num_hippos)
 */
void game_world_set_max_angle(game_world_t *world, double max_angle);

/**
 * game_world_set_max_speed sets the global maximum speed in the world to the
 * given parameter
 * @param world the game_world in which you are setting this value
 * @param max_speed the new maximum speed
 */
void game_world_set_max_speed(game_world_t *world, double max_speed);

/**
 * game_world_set_num_rounds sets the number of rounds in the world to the given
 * parameter
 * @param world the game_world in which you are setting this value
 * @param num_rounds the number of rounds the game should have
 */
void game_world_set_num_rounds(game_world_t *world, double num_rounds);

/**
 * game_world_set_arena_center sets the center of the arena in this game to the
 * given paramter
 * @param world the game_world in which you are setting this value
 * @param arena_center the new center of the arena
 */
void game_world_set_arena_center(game_world_t *world, vector_t arena_center);

/**
 * game_world_set_scene sets the scene of the game to the given paramter
 * @param world the game_world in which you are setting this value
 * @param scene the new scene for the game to use
 */
void game_world_set_scene(game_world_t *world, scene_t *scene);

/**
 * game_world_set_background sets the background of the game to the given
 * parameter
 * @param world the game_world in which you are setting this value
 * @param background the background for the game to use
 */
void game_world_set_background(game_world_t *world, body_t *background);

// FUNCTIONS

/**
 * game_world_add_hippo adds a new hippo to the list of pre-existing hippos
 * Also updates the number of hippos in the world
 * @param world the game_world in which you are adding this value
 * @param hippo the hippo you are adding to this world
 */
void game_world_add_hippo(game_world_t *world, player_t *hippo);

/**
 * game_world_add_ball adds a new ball to the list of pre-existing balls
 * Also updates the number of balls in the world
 * @param world the game_world in which you are adding this value
 * @param ball the ball you are adding to this world
 */
void game_world_add_ball(game_world_t *world, body_t *ball);

/**
 * game_world_remove_hippo removes a hippo from the list of hippos and returns
 * it Also updates the number of hippos in the world
 * @param world the game_world in which you are removing this value
 * @param hippo_to_be_removed the hippo you are removing from this world
 */
player_t *game_world_remove_hippo(game_world_t *world,
                                  player_t *hippo_to_be_removed);

/**
 * game_world_remove_ball removes a ball from the list of balls and returns it
 * Also updates the number of balls in the world
 * @param world the game_world in which you are removing this value
 * @param ball_to_be_removed the ball you are removing from this world
 */
body_t *game_world_remove_ball(game_world_t *world, body_t *ball_to_be_removed);

/**
 * game_world_decrement_round decrements the number of rounds left in the game
 * and returns the old number of rounds
 * @param world the game in which you are decrementing the round
 */
double game_world_decrement_round(game_world_t *world);

/**
 * TODO
 */
void game_world_increase_angle_modifier(game_world_t *world,
                                        size_t idx,
                                        double range);

/**
 * TODO
 */
void game_world_set_angle_modifier(game_world_t *world,
                                   size_t idx,
                                   double range);

/**
 * TODO
 */
void game_world_reset_arena(game_world_t *world,
                            double elasticity,
                            double max_angle,
                            double max_speed);

/**
 * frees the game and everything malloc'ed in game_world
 * note: does not free scene
 * @param world the game_world to be freed
 */
void game_world_free(game_world_t *world);

/**
 * Executes a tick of a given game_world over a small time interval.
 * This calls scene_tick for the scene stored in game_world
 * If any players are marked for removal, they should be removed from the scene
 * and freed, along with any force creators acting on them.
 * @param world a pointer to a game_world returned from game_world_init()
 * @param dt the time elapsed since the last tick, in seconds
 */
void game_world_tick(game_world_t *world, double dt);

#endif // #ifndef __GAME_WORLD_H__
