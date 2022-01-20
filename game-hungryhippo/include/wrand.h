#ifndef __WRAND_H__
#define __WRAND_H__

#include <stdlib.h>

/**
 * A random generator with a weighted distribution over a finite set of indices.
 * (No claims that this is at all reasonably random, just a quick and dirty
 * refactoring of the hungaryhippos random ball generator.)
 */
typedef struct wrand wrand_t;

/**
 * Create a new random generator. The generator will sample a random index from
 * [0,bin_count) weighted by 'weights', so weights must have length 'bin_count'.
 * The weights are doubles, and their total sum can be any positive finite
 * number. The generator uses 'rand' behind the scenes, and you are
 * responsible for seeding it using 'srand' before calling this function.
 * wrand does nothing to the weights array, so you should free it yourself if
 * necessary. Undefined behavior occurs if bin_count is nonpositive.
 */
wrand_t *wrand_init(size_t bin_count, const double *weights);

/**
 * Sample an index from [0,bin_count) randomly according to 'weights'.
 * If an error occured, then a size no less than 'bin_count' is returned.
 */
size_t wrand_sample(wrand_t *wrand);

/**
 * Free the randomizer along with its sample space.
 */
void wrand_free(wrand_t *wrand);

#endif // #ifnded __WRAND_H__
