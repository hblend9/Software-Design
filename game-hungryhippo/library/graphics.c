#include "graphics.h"
#include "list.h"
#include "sdl_wrapper.h"
#include "stdio.h"
#include "stdlib.h"
#include "text.h"
#include "vector.h"

/*** GLOBALS ***/

int _graphics_count = 0;

/*** TYPES ***/

struct graphics {
    vector_t dims; // Dimensions in scene coordiantes.
    list_t *body_groups;
    list_t *text_tab_groups;
    list_t *text_ln_groups;
};

/*** PRIVATE PROTOTYPES ***/
void _graphics_render_groups(list_t *groups, void (*rend_func)(void *));
void _graphics_render_objects(list_t *objects, void (*rend_func)(void *));

/*** DEFINITIONS ***/

graphics_t *graphics_init(vector_t dims) {
    if (_graphics_count > 0) {
        fprintf(stderr,
                "Invalid state: you cannot have multiple graphics layers.");
        exit(1);
    }
    graphics_t *graphics = malloc(sizeof(graphics_t));
    graphics->dims = dims;
    graphics->body_groups = list_init(1, NULL);
    graphics->text_tab_groups = list_init(1, NULL);
    graphics->text_ln_groups = list_init(1, NULL);
    _graphics_count++;
    return graphics;
}

void graphics_free(graphics_t *graphics) {
    list_free(graphics->text_tab_groups);
    list_free(graphics->text_ln_groups);
    list_free(graphics->body_groups);
    free(graphics);
    _graphics_count--;
}

void graphics_add_bodies(graphics_t *graphics, list_t *bodies) {
    list_add(graphics->body_groups, bodies);
}

void graphics_add_text_tabs(graphics_t *graphics, list_t *text_tabs) {
    list_add(graphics->text_tab_groups, text_tabs);
}

void graphics_add_text_lns(graphics_t *graphics, list_t *text_lns) {
    list_add(graphics->text_ln_groups, text_lns);
}

void graphics_render(graphics_t *graphics) {
    sdl_clear();
    _graphics_render_groups(graphics->body_groups,
                            (void (*)(void *))sdl_render_body);
    _graphics_render_groups(graphics->text_tab_groups,
                            (void (*)(void *))text_tab_render);
    _graphics_render_groups(graphics->text_ln_groups,
                            (void (*)(void *))text_ln_render);
    sdl_show();
}

void _graphics_render_groups(list_t *groups, void (*rend_func)(void *)) {
    for (size_t i = 0; i < list_size(groups); i++) {
        _graphics_render_objects(list_get(groups, i), rend_func);
    }
}

void _graphics_render_objects(list_t *objects, void (*rend_func)(void *)) {
    for (size_t i = 0; i < list_size(objects); i++) {
        rend_func(list_get(objects, i));
    }
}
