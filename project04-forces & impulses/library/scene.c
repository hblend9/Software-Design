#include "scene.h"
#include "body.h"
#include "list.h"
#include <assert.h>
#include <stdlib.h>

const size_t INITIAL_SIZE = 10;

typedef struct scene {
    list_t *bodies;
    vector_t min;
    vector_t max;

    double total_time;

    list_t *force_auxs;
} scene_t;

/**
 * Private struct to keep track of the force creators, their auxiliary
 * parameters, and the function to free aux.
 */
typedef struct force_aux {
    force_creator_t force_creator;
    free_func_t freer;

    void *aux;
} force_aux_t;

/*** Private function prototypes. ***/

force_aux_t *
force_aux_init(force_creator_t force_creator, free_func_t aux_freer, void *aux);
void force_aux_free(force_aux_t *fa);

/*** Private function definitions. ***/

force_aux_t *force_aux_init(force_creator_t force_creator,
                            free_func_t aux_freer,
                            void *aux) {
    force_aux_t *fa = malloc(sizeof(force_aux_t));
    assert(fa != NULL);

    fa->force_creator = force_creator;
    fa->freer = aux_freer;
    fa->aux = aux;

    return fa;
}

void force_aux_free(force_aux_t *fa) {
    fa->freer(fa->aux);

    free(fa);
}

/*** Public function definitions. ***/

scene_t *scene_init(void) {
    scene_t *scene = malloc(sizeof(scene_t));
    assert(scene != NULL);

    scene->bodies = list_init(INITIAL_SIZE, (free_func_t)body_free);
    // Dimensions to be set where the scene is used.
    scene_set_dims(scene, VEC_ZERO, VEC_ZERO);
    scene->total_time = 0;

    scene->force_auxs = list_init(1, (free_func_t)force_aux_free);

    return scene;
}

void scene_free(scene_t *scene) {
    list_free(scene->bodies);
    list_free(scene->force_auxs);
    free(scene);
}

size_t scene_bodies(scene_t *scene) {
    return list_size(scene->bodies);
}

body_t *scene_get_body(scene_t *scene, size_t index) {
    assert(index >= 0);
    return list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
    list_add(scene->bodies, body);
}

void scene_remove_body(scene_t *scene, size_t index) {
    body_free(list_remove(scene->bodies, index));
}

void scene_tick(scene_t *scene, double dt) {
    list_t *fas = scene->force_auxs;
    force_aux_t *fa_curr;
    for (size_t i = 0; i < list_size(fas); i++) {
        fa_curr = list_get(fas, i);
        fa_curr->force_creator(fa_curr->aux);
    }

    for (size_t j = 0; j < scene_bodies(scene); j++) {
        body_tick(scene_get_body(scene, j), dt);
    }

    scene->total_time += dt;
}

list_t *scene_get_bodies(scene_t *scene) {
    return scene->bodies;
}

void scene_set_dims(scene_t *scene, vector_t new_min, vector_t new_max) {
    scene->min = new_min;
    scene->max = new_max;
}

vector_t scene_get_min(scene_t *scene) {
    return scene->min;
}

vector_t scene_get_max(scene_t *scene) {
    return scene->max;
}

void scene_add_force_creator(scene_t *scene,
                             force_creator_t forcer,
                             void *aux,
                             free_func_t freer) {
    list_add(scene->force_auxs, force_aux_init(forcer, freer, aux));
}
