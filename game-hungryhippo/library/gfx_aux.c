#include "sdl_wrapper.h"

/*** STRUCTS ***/
struct gfx_aux {
    list_t *sprites;
    size_t active_sprite_idx;
};

/** DEFINITIONS OF PUBLIC FUNCTIONS ***/
gfx_aux_t *gfx_aux_init(list_t *sprites) {
    gfx_aux_t *gfx_aux = malloc(sizeof(gfx_aux_t));
    gfx_aux->sprites = sprites;
    gfx_aux->active_sprite_idx = 0;

    return gfx_aux;
}

void gfx_aux_setup(gfx_aux_t *gfx_aux, vector_t *anchor, double *angle) {
    list_t *sprites = gfx_aux->sprites;
    for (size_t i = 0; i < list_size(sprites); i++) {
        sprite_setup(list_get(sprites, i), anchor, angle);
    }
}

void gfx_aux_setup_with_body(gfx_aux_t *gfx_aux, body_t *body) {
    list_t *sprites = gfx_aux->sprites;
    for (size_t i = 0; i < list_size(sprites); i++) {
        sprite_setup_with_body(list_get(sprites, i), body);
    }
}

void gfx_aux_free(gfx_aux_t *gfx_aux) {
    list_free(gfx_aux->sprites);
    free(gfx_aux);
}

void gfx_aux_set_sprite(gfx_aux_t *gfx_aux, size_t idx) {
    if (idx < 0 || idx >= list_size(gfx_aux->sprites)) {
        fprintf(stderr, "Fatal error: cannot activate nonexistent sprite.\n");
        exit(1);
    }
    gfx_aux->active_sprite_idx = idx;
}

void gfx_aux_render(gfx_aux_t *gfx_aux) {
    sdl_render_sprite(list_get(gfx_aux->sprites, gfx_aux->active_sprite_idx));
}
