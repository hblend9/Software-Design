#include "sprite.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "vector.h"
#include <SDL2/SDL2_rotozoom.h>

/*** STRUCTS ***/

struct sprite {
    SDL_Texture *tex;
    /**
     * offset - A vector in scene coordinates that points from the anchor to
     * the top left corner of the sprite's rectangle.
     */
    vector_t offset;
    const vector_t *anchor;
    const double *angle;
    /**
     * dims - Dimensions of the sprite's rectangle in screen coordinates.
     */
    SDL_Point dims;
};

/*** DEFINITIONS OF PUBLIC FUNCTIONS ***/

sprite_t *sprite_init(const char *img_path, double scale, vector_t offset) {
    sprite_t *sprite = calloc(1, sizeof(sprite_t));
    SDL_Surface *surface_original = IMG_Load(img_path);
    sdl_handle_error("sprite_init: IMG_Load", surface_original != NULL);
    SDL_Surface *surface
        = rotozoomSurface(surface_original, 0, scale, SMOOTHING_ON);
    sprite->tex = SDL_CreateTextureFromSurface(sdl_get_renderer(), surface);
    SDL_FreeSurface(surface_original);
    SDL_FreeSurface(surface);
    sprite->anchor = NULL;
    sprite->angle = NULL;
    SDL_QueryTexture(sprite->tex,
                     NULL,
                     NULL,
                     &(sprite->dims.x),
                     &(sprite->dims.y));

    vector_t center_to_topleft_sce = vec_multiply(
        1.0 / sdl_sce_to_scr_scale() / 2.0,
        (vector_t){-(double)(sprite->dims.x), (double)(sprite->dims.y)});
    sprite->offset = vec_add(offset, center_to_topleft_sce);

    return sprite;
}

list_t *
sprite_init_many(const list_t *paths, double scale, const list_t *offsets) {
    assert(list_size(paths) > 0 && list_size(paths) == list_size(offsets));
    list_t *sprites = list_init(list_size(paths), (free_func_t)sprite_free);
    for (size_t i = 0; i < list_size(paths); i++) {
        list_add(sprites,
                 sprite_init(list_get(paths, i),
                             scale,
                             *(vector_t *)list_get(offsets, i)));
    }
    return sprites;
}

void sprite_setup(sprite_t *sprite,
                  const vector_t *anchor,
                  const double *angle) {
    sprite->anchor = anchor;
    sprite->angle = angle;
}

void sprite_setup_with_body(sprite_t *sprite, body_t *body) {
    sprite_setup(sprite, body_get_anchor(body), body_get_angle_p(body));
    // We assume that the zeroth shape of the body is its best graphical
    // approximation.
    sprite->offset
        = vec_add(sprite->offset,
                  polygon_centroid_to_center(body_get_shape_alt(body, 0)));
}

void sprite_free(sprite_t *sprite) {
    SDL_DestroyTexture(sprite->tex);
    free(sprite);
}

void sprite_render(sprite_t *sprite) {
    const vector_t *anchor = sprite->anchor;
    vector_t offset = sprite->offset;
    SDL_Rect *rect = malloc(sizeof(SDL_Rect));
    SDL_Point *pivot = malloc(sizeof(SDL_Point));

    // Convert anchor to screen coordinates for later.
    SDL_Point anchor_scr = sdl_sce_to_scr_coord(*anchor);
    // The topleft of the rect in scene coordinates before rotation
    // (SDL_RenderCopyEx handles rotation).
    // rect_center = anchor + offset
    vector_t rect_topleft_sce = vec_add(*anchor, offset);
    // Convert to screen coodinates.
    SDL_Point rect_topleft_scr = sdl_sce_to_scr_coord(rect_topleft_sce);

    // Convert angle to screen degrees.
    double angle_scr = sdl_sce_to_scr_angle(*(sprite->angle));

    // Populate rectangle.
    rect->w = sprite->dims.x;
    rect->h = sprite->dims.y;
    rect->x = rect_topleft_scr.x;
    rect->y = rect_topleft_scr.y;

    // Compute pivot relative to top left corner.
    // Abstractly, the pivot is literally the negative of the offset, but this
    // seems the simplest way to convert from scene to screen coordinates.
    pivot->x = anchor_scr.x - rect_topleft_scr.x;
    pivot->y = anchor_scr.y - rect_topleft_scr.y;

    SDL_RenderCopyEx(sdl_get_renderer(),
                     sprite->tex,
                     NULL,
                     rect,
                     angle_scr,
                     pivot,
                     SDL_FLIP_NONE);

    free(rect);
    free(pivot);
}
