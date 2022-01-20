#ifndef __EHHH_H__
#define __EHHH_H__

#include <stdbool.h>
#include <stdlib.h>

/*** DEPENDENCY FORWARD DECLARATIONS ***/
typedef struct vector vector_t;
typedef struct list list_t;
typedef struct game game_t;
typedef struct text_ln text_ln_t;

/*** INTERFACE ***/

#define EHHH_MAX_PLAYERS 4
#define EHHH_MIN_PLAYERS 2
extern const size_t EHHH_MAX_BALLS_PER_ROUND;

/**
 * An instance of Extremely Hungry Hungry Hippos, the game.
 */
typedef struct ehhh ehhh_t;

/**
 * Initialize the game with players_count many players and graphics with 'dims'.
 */
ehhh_t *ehhh_init(size_t player_count, size_t balls_per_round);

/**
 * Tick the game forward dt seconds.
 * This means tick the physics and graphics, collect any garbage, and handle key
 * presses.
 * Return true if the game is over, false otherwise.
 */
bool ehhh_tick(ehhh_t *ehhh, double dt);

/**
 * Free the game and all associated data.
 */
void ehhh_free(ehhh_t *ehhh);

/**
 * Get *the* list of player bodies. Don't free and probably don't remove from
 * this list.
 */
list_t *ehhh_get_players(ehhh_t *ehhh);

/**
 * Get *the* list of ball bodies. Don't free and probably don't remove from this
 * list.
 */
list_t *ehhh_get_balls(ehhh_t *ehhh);

/**
 * Get the game.
 */
game_t *ehhh_get_game(ehhh_t *ehhh);

/**
 * Get the instance's elasticity.
 */
double ehhh_get_elasticity(ehhh_t *ehhh);

double ehhh_get_max_speed(ehhh_t *ehhh);
double ehhh_get_max_angle(ehhh_t *ehhh);

void ehhh_set_max_speed(ehhh_t *ehhh, double value);

vector_t ehhh_get_center(ehhh_t *ehhh);
double ehhh_get_radius(ehhh_t *ehhh);

/**
 * Splash some text on the screen.
 * Return the resulting text_ln, which can be marked for removal or whose text
 * can be updated as desired via the text interface.
 */
text_ln_t *ehhh_splash(ehhh_t *ehhh, char *str);

#endif // #ifndef __EHHH_H__
