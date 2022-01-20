/**
 * NOTES
 *  Generally functions that return pointers do *not* return copies, so the
 *  result doesn't need to be freed (since `player_free` will do that). If a
 *  function returns a copy, then it will say so.
 */

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "ball.h"
#include "body.h"
#include "list.h"
#include "text.h"
#include <stdbool.h>

/**
 * Possible states of a player, which determines its behavior and appearance.
 */
typedef enum player_state {
    PLAYER_EATING,  // Trying to eat.
    PLAYER_CHILLING // Not eating but may be moving around.
} player_state_e;

typedef enum player_shape_idx {
    PLAYER_HIPPO_BODY_NOT_EATING_IDX,
    PLAYER_HIPPO_BODY_EATING_IDX,
    PLAYER_HIPPO_MOUTH_IDX,
    PLAYER_NUM_SHAPES
} player_shape_idx_e;

typedef enum player_sprite {
    PLAYER_SPRITE_CHILLING,
    PLAYER_SPRITE_EATING,
    PLAYER_SPRITE_COUNT
} player_sprite_t;

/**
 * A player, with a body and a state.
 */
typedef struct player player_t;

typedef struct powerup powerup_t;

typedef struct ehhh ehhh_t;

/**
 * Initialize a player associated with body.
 * @param idx The player index (player number, zero-indexed).
 * @param origin The player's initial position, which is accessible later via
 * 'player_get_origin'.
 */
player_t *player_init(size_t idx, vector_t origin);

/**
 * Free a player (but not its body, because the player belongs to the body).
 */
void player_free(player_t *player);

/**
 * Get the player's index.
 */
size_t player_get_idx(player_t *player);

/**
 * Get the player's body (consensually).
 */
body_t *player_get_body(player_t *player);

/**
 * Return the list of powerups associated with the player (not a copy, so don't
 * modify unless you actually mean to).
 */
list_t *player_get_powerups(player_t *player);

/**
 * Return the idx'th powerup of player.
 * This powerup is still handled by player, so no need to free.
 * If the idx'th powerup does not exist, return NULL.
 */
powerup_t *player_get_powerup(player_t *player, size_t idx);

/**
 * Return the idx'th powerup of player and remove it from the player, so it
 * needs to be freed.
 */
powerup_t *player_remove_powerup(player_t *player, size_t idx);

/**
 * Get the current state of the player.
 */
player_state_e player_get_state(player_t *player);

/**
 * Set the current state of the player. Adjusts internal state of the player
 * if needed.
 */
void player_set_state(player_t *player, player_state_e state);

/**
 * Add a powerup to the player's list.
 */
void player_add_powerup(player_t *player, powerup_t *powerup);

/**
 * Activiate the idx'th powerup of the player and free it.
 */
void player_activate_powerup(player_t *player, ehhh_t *ehhh);

/**
 * Return and remove the currently selected powerup if it exists or NULL if it
 * does not.
 */
powerup_t *player_get_curr_powerup(player_t *player);

/**
 * Returns the current powerup index.
 */
size_t player_get_powerup_idx(player_t *player);

/**
 * Increment the current powerup index by amount, to select a different powerup.
 */
void player_inc_powerup_idx(player_t *player, size_t amount);

/**
 * Add 'points' number of points to the player. The amount can be negative.
 */
void player_add_points(player_t *player, short points);

/**
 * Return the points a player has.
 */
short player_get_points(player_t *player);

/**
 * Mark the player and its body for deffered removal.
 */
void player_remove(player_t *player);

/**
 * Returns the address of the hippo's (player's) current body shape.
 */
list_t **player_get_hippo_body_shape(player_t *player);

/**
 * Returns the address of the hippo's (player's) current mouth shape.
 */
list_t **player_get_hippo_mouth_shape(player_t *player);

/**
 * Returns whether the player is in the eating state.
 */
bool player_is_eating(player_t *player);

/**
 * Returns whether the player should be removed on this tick
 */
bool player_is_removed(player_t *player);

/**
 * Adds a factor to the current speed factor.
 */
void player_add_speed_factor(player_t *player, double factor);

/**
 * Sets a multiplicative speed factor for the player.
 */
void player_set_speed_factor(player_t *player, double factor);

/**
 * Returns the player's angular speed multiplied by any set speed factor.
 */
double player_get_speed_factor(player_t *player);

/**
 * Sets 'range' as the player's current angle modifier.
 */
void player_set_angle_modifier(player_t *player, double range);

/**
 * Adds 'range' to the player's current angle modifier.
 */
void player_add_angle_modifier(player_t *player, double range);

/**
 * Returns the player's angle modifier in radians.
 */
double player_get_angle_modifier(player_t *player);

/**
 * Returns the coordinate pair of the location to spawn a ball by centroid
 * when shooting.
 */
vector_t player_get_shoot_location(player_t *player, ehhh_t *ehhh);

/**
 * Returns the velocity to spawn a ball at when shooting, relative to the
 * shoot location, mouth of the hippo, and center of the arena.
 */
vector_t player_get_shoot_velocity(player_t *player, ehhh_t *ehhh);

/**
 * Create a text table of the players' stats.
 * Each player gets a row, and each row has the following format:
 *      P[player index]  +[points]  [powerup count]>[current powerup]
 * @param topleft Position of topleft corner of the table.
 */
text_tab_t *player_create_tab(list_t *players, vector_t topleft);

/**
 * Get the player's initial position.
 */
vector_t player_get_origin(player_t *player);

#endif // #ifndef __PLAYER_H__
