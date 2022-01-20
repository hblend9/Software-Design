#include "physics.h"
#include "body.h"
#include "forces.h"
#include "list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/*** STRUCTURES ***/

struct physics {
    list_t *body_groups;
    list_t *force_trackers;
};

/**
 * Private struct to keep track of the force creators, their auxiliary
 * parameters, and the function to free aux.
 */
typedef struct _force_tracker {
    force_creator_t force_creator;
    free_func_t freer;
    void *aux;
    list_t *bodies; // Which bodies associated with this force creator.
} _force_tracker_t;

/*** PRIVATE FUNCTION PROTOTYPES ***/

/**
 * Init a force tracker.
 */
_force_tracker_t *_force_tracker_init(force_creator_t force_creator,
                                      free_func_t aux_freer,
                                      void *aux,
                                      list_t *bodies);

/**
 * Free a force tracker but not its associated bodies.
 */
void _force_tracker_free(_force_tracker_t *fa);

/**
 * Return true if the force tracker has been marked for removal (i.e. any of its
 * bodies have been so.
 */
bool _force_tracker_is_removed(_force_tracker_t *fa);

/**
 * Collect garbage, i.e. remove any forces whose bodies have been marked for it.
 */
void _physics_collect_garbage(physics_t *physics);

/*** DEFINITIONS ***/

_force_tracker_t *_force_tracker_init(force_creator_t force_creator,
                                      free_func_t aux_freer,
                                      void *aux,
                                      list_t *bodies) {
    _force_tracker_t *fa = malloc(sizeof(_force_tracker_t));
    assert(fa != NULL);

    fa->force_creator = force_creator;
    fa->freer = aux_freer;
    fa->aux = aux;
    fa->bodies = bodies;

    return fa;
}

void _force_tracker_free(_force_tracker_t *fa) {
    if (fa->freer != NULL && fa->aux != NULL) {
        fa->freer(fa->aux);
    }
    list_free(fa->bodies);
    free(fa);
}

bool _force_tracker_is_removed(_force_tracker_t *fa) {
    for (size_t i = 0; i < list_size(fa->bodies); i++) {
        if (body_is_removed(list_get(fa->bodies, i))) {
            return true;
        }
    }
    return false;
}

physics_t *physics_init(void) {
    physics_t *physics = malloc(sizeof(physics_t));
    assert(physics != NULL);
    physics->body_groups = list_init(1, NULL);
    physics->force_trackers = list_init(1, (free_func_t)_force_tracker_free);
    return physics;
}

void physics_free(physics_t *physics) {
    list_free(physics->body_groups);
    list_free(physics->force_trackers);
    free(physics);
}

void physics_add_bodies(physics_t *physics, list_t *bodies) {
    assert(bodies != NULL);
    list_add(physics->body_groups, bodies);
}

void physics_add_force(physics_t *physics,
                       force_creator_t forcer,
                       void *aux,
                       list_t *bodies,
                       free_func_t freer) {
    list_add(physics->force_trackers,
             _force_tracker_init(forcer, freer, aux, bodies));
}

void physics_tick(physics_t *physics, double dt) {
    // Tick forces
    list_t *fas = physics->force_trackers;
    _force_tracker_t *fa_curr;
    for (int i = list_size(fas) - 1; i >= 0; i--) {
        fa_curr = list_get(fas, i);
        fa_curr->force_creator(fa_curr->aux);
    }
    // Tick bodies.
    for (int i = 0; i < list_size(physics->body_groups); i++) {
        list_t *bodies = list_get(physics->body_groups, i);
        for (int j = list_size(bodies) - 1; j >= 0; j--) {
            body_tick(list_get(bodies, j), dt);
        }
    }
    // Collect garbage.
    _physics_collect_garbage(physics);
}

void _physics_collect_garbage(physics_t *physics) {
    list_t *fas = physics->force_trackers;
    _force_tracker_t *fa_curr;
    for (int i = list_size(fas) - 1; i >= 0; i--) {
        fa_curr = list_get(fas, i);
        if (_force_tracker_is_removed(fa_curr)) {
            _force_tracker_free(list_remove(fas, i));
        }
    }
}
