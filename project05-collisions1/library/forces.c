#include "forces.h"
#include "body.h"
#include "list.h"
#include "scene.h"
#include <assert.h>
#include <collision.h>
#include <stdlib.h>

// The minimum "pixel distance" to compute Newtonian gravity at.
const double GRAVITY_MIN_DISTANCE_THRESHOLD = 5.0;

/*** Private function prototypes. ***/

// Force creator functions.
void force_creator_newtonian_gravity(aux_t *aux);
void force_creator_spring(aux_t *aux);
void force_creator_drag(aux_t *aux);
void force_creator_destructive_collision(aux_t *aux);

/*** Public function definitions. ***/

aux_t *aux_init(size_t n_consts, size_t n_bodies) {
    aux_t *aux = malloc(sizeof(aux_t));
    assert(aux != NULL);

    // Set the initial sizes to save memory.
    // Bodies should be freed elsewhere, so pass NULL.
    aux->bodies = list_init(n_bodies, NULL);
    // Will free the constants automatically since they are added "here".
    aux->constants = list_init(n_consts, free);

    return aux;
}

void aux_free(aux_t *aux) {
    list_free(aux->bodies);
    list_free(aux->constants);
    free(aux);
}

void aux_add_body(aux_t *aux, body_t *body) {
    list_add(aux->bodies, body);
}

void aux_add_constant(aux_t *aux, double c) {
    double *c_p = malloc(sizeof(double));
    assert(c_p != NULL);

    *c_p = c;

    list_add(aux->constants, c_p);
}

void create_newtonian_gravity(scene_t *scene,
                              double G,
                              body_t *body1,
                              body_t *body2) {
    aux_t *aux = aux_init(1, 2);

    aux_add_constant(aux, G);
    aux_add_body(aux, body1);
    aux_add_body(aux, body2);

    scene_add_bodies_force_creator(
        scene,
        (force_creator_t)force_creator_newtonian_gravity,
        aux,
        list_copy(aux->bodies, NULL),
        (free_func_t)aux_free);
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
    aux_t *aux = aux_init(2, 2);
    // Constant for the initial, equilibrium distance between the centroids of
    // the bodies.
    double eq_dist = 0;
    // Alternatively use initial distance:
    // vec_magnitude(vec_subtract(body_get_centroid(body1),
    // body_get_centroid(body2)));

    // The spring constant, k, is the first constant added.
    aux_add_constant(aux, k);
    aux_add_constant(aux, eq_dist);
    aux_add_body(aux, body1);
    aux_add_body(aux, body2);

    scene_add_bodies_force_creator(scene,
                                   (force_creator_t)force_creator_spring,
                                   aux,
                                   list_copy(aux->bodies, NULL),
                                   (free_func_t)aux_free);
}

void create_drag(scene_t *scene, double gamma, body_t *body) {
    aux_t *aux = aux_init(1, 1);

    aux_add_constant(aux, gamma);
    aux_add_body(aux, body);

    scene_add_bodies_force_creator(scene,
                                   (force_creator_t)force_creator_drag,
                                   aux,
                                   list_copy(aux->bodies, NULL),
                                   (free_func_t)aux_free);
}

void create_destructive_collision(scene_t *scene,
                                  body_t *body1,
                                  body_t *body2) {
    aux_t *aux = aux_init(0, 2);

    aux_add_body(aux, body1);
    aux_add_body(aux, body2);

    scene_add_bodies_force_creator(
        scene,
        (force_creator_t)force_creator_destructive_collision,
        aux,
        list_copy(aux->bodies, NULL),
        (free_func_t)aux_free);
}

/*** Private function definitions. ***/

void force_creator_newtonian_gravity(aux_t *aux) {
    body_t *b1 = (body_t *)list_get(aux->bodies, 0);
    body_t *b2 = (body_t *)list_get(aux->bodies, 1);

    // Vector from centroid of b1 to centroid of b2.
    vector_t r_21 = vec_subtract(body_get_centroid(b2), body_get_centroid(b1));

    double mag_r_21 = vec_magnitude(r_21);

    // Force vectors.
    vector_t f_21 = VEC_ZERO;
    vector_t f_12 = VEC_ZERO;

    if (mag_r_21 > GRAVITY_MIN_DISTANCE_THRESHOLD) {
        double m1 = body_get_mass(b1);
        double m2 = body_get_mass(b2);
        double G = *(double *)list_get(aux->constants, 0);

        // This is the Newtonian gravitational force.
        f_21 = vec_multiply(-G * m1 * m2 / (mag_r_21 * mag_r_21 * mag_r_21),
                            r_21);
        // Need equal and opposite forces, one for each object.
        f_12 = vec_negate(f_21);
    }

    body_add_force(b1, f_12);
    body_add_force(b2, f_21);
}

void force_creator_spring(aux_t *aux) {
    body_t *b1 = (body_t *)list_get(aux->bodies, 0);
    body_t *b2 = (body_t *)list_get(aux->bodies, 1);

    // Vector from centroid of b1 to centroid of b2 (the direction we want b1 to
    // go in, opposite for b2).
    vector_t r_21 = vec_subtract(body_get_centroid(b2), body_get_centroid(b1));

    double mag_r_21 = vec_magnitude(r_21);
    double equilibrium_distance = *(double *)list_get(aux->constants, 1);
    double k = *(double *)list_get(aux->constants, 0);

    // Force vectors.
    // F = k * x, in the direction of the centroid differences as a unit vector.
    vector_t f_1 = vec_multiply(k * (mag_r_21 - equilibrium_distance),
                                vec_multiply(1.0 / mag_r_21, r_21));
    vector_t f_2 = vec_negate(f_1);

    body_add_force(b1, f_1);
    body_add_force(b2, f_2);
}

void force_creator_drag(aux_t *aux) {
    body_t *b = (body_t *)list_get(aux->bodies, 0);

    vector_t v = body_get_velocity(b);
    double gamma = 0;

    gamma = *(double *)list_get(aux->constants, 0);

    body_add_force(b, vec_multiply(-gamma, v));
}

void force_creator_destructive_collision(aux_t *aux) {
    body_t *body0 = (body_t *)list_get(aux->bodies, 0);
    body_t *body1 = (body_t *)list_get(aux->bodies, 1);
    list_t *shape0 = body_get_shape(body0);
    list_t *shape1 = body_get_shape(body1);
    if (find_collision(shape0, shape1)) {
        body_remove(body0);
        body_remove(body1);
    }
    list_free(shape0);
    list_free(shape1);
}
