#ifndef __GFX_AUX_H__
#define __GFX_AUX_H__

#include "body.h"
#include "list.h"
#include "vector.h"

typedef struct gfx_aux gfx_aux_t;

// Defined by body.h; provided to avoid circular includes.
typedef struct body body_t;

gfx_aux_t *gfx_aux_init(list_t *sprites);

void gfx_aux_setup(gfx_aux_t *gfx_aux, vector_t *anchor, double *angle);

void gfx_aux_setup_with_body(gfx_aux_t *gfx_aux, body_t *body);

void gfx_aux_free(gfx_aux_t *gfx_aux);

/**
 * Set the active sprite, addressed by index.
 */
void gfx_aux_set_sprite(gfx_aux_t *gfx_aux, size_t idx);

void gfx_aux_render(gfx_aux_t *gfx_aux);

#endif // #ifndef __GFX_AUX_H__
