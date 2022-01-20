#ifndef __BALL_H__
#define __BALL_H__

#include "body.h"
#include "player.h"
#include "vector.h"

#define THOUSAND 1000
extern const double BALL_SHOOT_SPEED;
extern const size_t FIFTEEN_SECM;
extern const size_t TEN_SECM;

typedef enum BALL_POWER_TYPE {
    //  generic type
    POWER_NONE,
    // on eat effect types
    POWER_PLAYER_EAT_LO,
    POWER_PLAYER_EAT_KILL = POWER_PLAYER_EAT_LO,
    POWER_PLAYER_EAT_FAST,
    POWER_PLAYER_EAT_SLOW,
    POWER_PLAYER_EAT_SPILL,
    POWER_PLAYER_EAT_HI,
    // player activate powerup types
    POWER_PLAYER_ACTIVATE_LO = POWER_PLAYER_EAT_HI,
    POWER_PLAYER_ACTIVATE_FAST = POWER_PLAYER_ACTIVATE_LO,
    POWER_PLAYER_ACTIVATE_MORE_ANGLE,
    POWER_PLAYER_ACTIVATE_WORLD_FULL_ANGLE,
    POWER_PLAYER_ACTIVATE_WORLD_MORE_ANGLE,
    POWER_PLAYER_ACTIVATE_WORLD_RESET_ARENA,
    POWER_PLAYER_ACTIVATE_WORLD_FAST,
    POWER_PLAYER_ACTIVATE_HI,
    // shooting powerup types, will spawn an on_eat effect on player activate
    POWER_PLAYER_SHOOT_LO = POWER_PLAYER_ACTIVATE_HI,
    POWER_PLAYER_SHOOT_KILL = POWER_PLAYER_SHOOT_LO,
    POWER_PLAYER_SHOOT_SLOW,
    POWER_PLAYER_SHOOT_SPILL,
    POWER_PLAYER_SHOOT_HI,
    // POWER_PLAYER_SHOOT_DESTROY_BALL - might not be possible, want to
    // destroy a ball in play
    POWER_NUM_TYPES = POWER_PLAYER_SHOOT_HI
} ball_power_type_e;

/**
 * Cryptic string representations of the powers, indexed by ball_power_type_e.
 */
extern const char *POWER_STRS[];

typedef struct powerup powerup_t;

typedef struct player player_t;

typedef struct ehhh ehhh_t;

/**
 * Add a ball to world.
 */
void spawn_ball(ehhh_t *ehhh,
                vector_t init_pos,
                vector_t init_vel,
                ball_power_type_e type);

void powerup_free(powerup_t *powerup);

void powerup_activate(powerup_t *powerup, ehhh_t *ehhh, player_t *player);

void powerup_eat(powerup_t *powerup, ehhh_t *ehhh, player_t *player);

powerup_t *powerup_init(ball_power_type_e type);

/**
 * Return a string specifying the powerup which will need to be freed
 * by the caller.
 */
char *powerup_str(powerup_t *powerup);

#endif // #ifnded __BALL_H__
