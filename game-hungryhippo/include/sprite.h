#ifndef __SPRITE_H__
#define __SPRITE_H__

#include "body.h"
#include "list.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>

/**
 * A graphical sprite to be rended by SDL. Keeps track of linear and angular
 * pointers and provides a rendering interface.
 */
typedef struct sprite sprite_t;

/**
 * Initialize, but do not setup, a sprite. A sprite needs to be passed to
 * 'sprite_setup' before use.
 * @param offset Vector pointing from the anchor to center* of the sprite's
 * rectangle, before any rotation (rotation and translation is handled by
 * 'sprite_render'.
 *
 * *NOTE: If you use 'sprite_setup_with_body' afterwards, then the provided
 * offset is interpreted as pointing from the centroid of the body to the
 * centroid of the sprite; no need to compute this additional offset yourself.
 */
sprite_t *sprite_init(const char *img_path, double scale, vector_t offset);

/**
 * Return a list of sprites initalized from the list of paths and the
 * cooresonding list of offsets.
 *
 * All sprites will be initialized with the same scaling. If this is a problem,
 * either resize the collection of images so they can use the same scaling or
 * let Noah know that he needs to change this interface.
 * The offsets, however, can be different for each path. The offsets and paths
 * lists must have the same size.
 */
list_t *
sprite_init_many(const list_t *paths, double scale, const list_t *offsets);

/**
 * Associate with a sprite its anchor and angle pointers.
 */
void sprite_setup(sprite_t *sprite,
                  const vector_t *anchor,
                  const double *angle);

/**
 * Setup the sprite with a body.
 * This function supersedes, i.e. wraps, sprite_setup; no need to run both.
 * Namely, this function modifies the sprite's offset to point from centroid to
 * centroid. This fixes the problem where sometimes the centroid is not the same
 * as rectangular center.
 */
void sprite_setup_with_body(sprite_t *sprite, body_t *body);

void sprite_free(sprite_t *sprite);

void sprite_render(sprite_t *sprite);

#endif // #ifndef __SPRITE_H__
