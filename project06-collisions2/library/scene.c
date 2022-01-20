#include "scene.h"
#include "body.h"
#include "list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

const size_t INITIAL_SIZE = 10;

typedef struct scene {
    list_t *bodies;
    vector_t min;
    vector_t max;

    double total_time;

    list_t *force_trackers;
} scene_t;

/**
 * Private struct to keep track of the force creators, their auxiliary
 * parameters, and the function to free aux.
 */
typedef struct force_tracker {
    force_creator_t force_creator;
    free_func_t freer;
    void *aux;
    list_t *bodies; // Which bodies associated with this force creator.
} force_tracker_t;

/*** Private function prototypes. ***/

force_tracker_t *force_tracker_init(force_creator_t force_creator,
                                    free_func_t aux_freer,
                                    void *aux,
                                    list_t *bodies);
void force_tracker_free(force_tracker_t *fa);

/*** Private function definitions. ***/

force_tracker_t *force_tracker_init(force_creator_t force_creator,
                                    free_func_t aux_freer,
                                    void *aux,
                                    list_t *bodies) {
    force_tracker_t *fa = malloc(sizeof(force_tracker_t));
    assert(fa != NULL);

    fa->force_creator = force_creator;
    fa->freer = aux_freer;
    fa->aux = aux;
    fa->bodies = bodies;

    return fa;
}

void force_tracker_free(force_tracker_t *fa) {
    if (fa->freer != NULL && fa->aux != NULL) {
        fa->freer(fa->aux);
    }
    list_free(fa->bodies);
    free(fa);
}

bool force_tracker_is_removed(force_tracker_t *fa) {
    for (size_t i = 0; i < list_size(fa->bodies); i++) {
        if (body_is_removed(list_get(fa->bodies, i))) {
            return true;
        }
    }
    return false;
}

/*** Public function definitions. ***/

scene_t *scene_init(void) {
    scene_t *scene = malloc(sizeof(scene_t));
    assert(scene != NULL);

    scene->bodies = list_init(INITIAL_SIZE, (free_func_t)body_free);
    // Dimensions to be set where the scene is used.
    scene_set_dims(scene, VEC_ZERO, VEC_ZERO);
    scene->total_time = 0;

    scene->force_trackers = list_init(1, (free_func_t)force_tracker_free);

    return scene;
}

void scene_free(scene_t *scene) {
    list_free(scene->bodies);
    list_free(scene->force_trackers);
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

/* Deprecated. */
void scene_remove_body(scene_t *scene, size_t index) {
    /* body_free(list_remove(scene->bodies, index)); */
    body_remove(scene_get_body(scene, index));
}

void scene_tick(scene_t *scene, double dt) {
    list_t *fas = scene->force_trackers;
    force_tracker_t *fa_curr;

    // Tick forces
    for (int i = list_size(fas) - 1; i >= 0; i--) {
        fa_curr = list_get(fas, i);
        fa_curr->force_creator(fa_curr->aux);
    }
    // Tick bodies.
    for (int j = scene_bodies(scene) - 1; j >= 0; j--) {
        body_t *b = scene_get_body(scene, j);
        body_tick(b, dt);
    }
    // Remove forces if needed.
    for (int i = list_size(fas) - 1; i >= 0; i--) {
        fa_curr = list_get(fas, i);
        if (force_tracker_is_removed(fa_curr)) {
            force_tracker_free(list_remove(fas, i));
        }
    }
    // Remove bodies if needed.
    for (int j = scene_bodies(scene) - 1; j >= 0; j--) {
        body_t *b = scene_get_body(scene, j);
        if (body_is_removed(b)) {
            body_free(list_remove(scene->bodies, j));
        }
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

/* Deprecated. */
void scene_add_force_creator(scene_t *scene,
                             force_creator_t forcer,
                             void *aux,
                             free_func_t freer) {
    scene_add_bodies_force_creator(scene,
                                   forcer,
                                   aux,
                                   list_init(0, free),
                                   freer);
}

void scene_add_bodies_force_creator(scene_t *scene,
                                    force_creator_t forcer,
                                    void *aux,
                                    list_t *bodies,
                                    free_func_t freer) {
    list_add(scene->force_trackers,
             force_tracker_init(forcer, freer, aux, bodies));
}
