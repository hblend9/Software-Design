#include "forces.h"
#include "body.h"
#include "list.h"
#include "physics.h"
#include <assert.h>
#include <collision.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// The minimum "pixel distance" to compute Newtonian gravity at.
const double GRAVITY_MIN_DISTANCE_THRESHOLD = 5.0;

/*** Private function prototypes. ***/

// Force creator functions.
void force_creator_newtonian_gravity(aux_t *aux);
void force_creator_spring(aux_t *aux);
void force_creator_drag(aux_t *aux);
void force_creator_collision(aux_collision_t *aux);
void force_creator_collision_shapes(aux_collision_t *aux);
void force_creator_collision_shape(aux_collision_t *aux);
void collision_handler_destructive(body_t *body1,
                                   body_t *body2,
                                   vector_t axis,
                                   vector_t collision_point,
                                   aux_t *aux);
// an easier to read alias for create_asymetric destructive collision

void collision_handler_asymmetric_destructive(body_t *body_to_remove,
                                              body_t *body_other,
                                              vector_t axis,
                                              vector_t collision_point,
                                              aux_t *aux);
// an easier to read alias for create_physics collision

void collision_handler_physics(body_t *body1,
                               body_t *body2,
                               vector_t axis,
                               vector_t collision_point,
                               aux_t *aux);
void collision_handler_physics_spin(body_t *body1,
                                    body_t *body2,
                                    vector_t axis,
                                    vector_t collision_point,
                                    aux_t *aux);

/*** Public function definitions. ***/

aux_t *aux_init(size_t n_consts, size_t n_bodies) {
    aux_t *aux = malloc(sizeof(aux_t));
    assert(aux != NULL);

    aux->type = GENERAL_AUX;
    // Set the initial sizes to save memory.
    // Bodies should be freed elsewhere, so pass NULL.
    aux->bodies = list_init(n_bodies, NULL);
    // Will free the constants automatically since they are addedhere".
    aux->constants = list_init(n_consts, free);

    return aux;
}

aux_collision_t *aux_collision_init(size_t n_bodies,
                                    collision_handler_t handler,
                                    void *handler_aux,
                                    free_func_t handler_aux_freer) {
    aux_collision_t *aux = malloc(sizeof(aux_collision_t));
    assert(aux != NULL);

    aux->type = COLLISION_AUX;
    aux->bodies = list_init(n_bodies, NULL);
    aux->handler = handler;
    aux->handler_aux = handler_aux;
    aux->handler_aux_freer = handler_aux_freer;
    aux->already_collided = false;

    return aux;
}

// Should work for both AUX types.
void aux_free(aux_t *aux) {
    if (aux != NULL) {
        list_free(aux->bodies);
        aux_collision_t *aux_c;
        switch (aux->type) {
        case GENERAL_AUX:
            list_free(aux->constants);
            break;
        case COLLISION_AUX:
            aux_c = (aux_collision_t *)aux;
            if (aux_c->handler_aux_freer != NULL
                && aux_c->handler_aux != NULL) {
                aux_c->handler_aux_freer(aux_c->handler_aux);
            }
            break;
        default:
            break;
        }
        free(aux);
    }
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

void create_newtonian_gravity(physics_t *physics,
                              double G,
                              body_t *body1,
                              body_t *body2) {
    aux_t *aux = aux_init(1, 2);

    aux_add_constant(aux, G);
    aux_add_body(aux, body1);
    aux_add_body(aux, body2);

    physics_add_force(physics,
                      (force_creator_t)force_creator_newtonian_gravity,
                      aux,
                      list_copy(aux->bodies, NULL),
                      (free_func_t)aux_free);
}

void create_spring(physics_t *physics, double k, body_t *body1, body_t *body2) {
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

    physics_add_force(physics,
                      (force_creator_t)force_creator_spring,
                      aux,
                      list_copy(aux->bodies, NULL),
                      (free_func_t)aux_free);
}

void create_drag(physics_t *physics, double gamma, body_t *body) {
    aux_t *aux = aux_init(1, 1);

    aux_add_constant(aux, gamma);
    aux_add_body(aux, body);

    physics_add_force(physics,
                      (force_creator_t)force_creator_drag,
                      aux,
                      list_copy(aux->bodies, NULL),
                      (free_func_t)aux_free);
}

void create_collision(physics_t *physics,
                      body_t *body1,
                      body_t *body2,
                      collision_handler_t handler,
                      void *aux,
                      free_func_t freer) {
    create_collision_general(physics,
                             body1,
                             body2,
                             NULL,
                             NULL,
                             handler,
                             aux,
                             freer,
                             (force_creator_t)force_creator_collision);
}

void create_collision_shapes(physics_t *physics,
                             body_t *body1,
                             body_t *body2,
                             list_t **shape1_p,
                             list_t **shape2_p,
                             collision_handler_t handler,
                             void *aux,
                             free_func_t freer) {
    if (shape1_p == NULL) {
        if (shape2_p == NULL) {
            create_collision_general(physics,
                                     body1,
                                     body2,
                                     NULL,
                                     NULL,
                                     handler,
                                     aux,
                                     freer,
                                     (force_creator_t)force_creator_collision);

        } else {
            create_collision_general(
                physics,
                body2,
                body1,
                shape2_p,
                NULL,
                handler,
                aux,
                freer,
                (force_creator_t)force_creator_collision_shape);
        }
    } else if (shape2_p == NULL) {
        create_collision_general(
            physics,
            body1,
            body2,
            shape1_p,
            NULL,
            handler,
            aux,
            freer,
            (force_creator_t)force_creator_collision_shape);
    } else {
        create_collision_general(
            physics,
            body1,
            body2,
            shape1_p,
            shape2_p,
            handler,
            aux,
            freer,
            (force_creator_t)force_creator_collision_shapes);
    }
}

void create_collision_general(physics_t *physics,
                              body_t *body1,
                              body_t *body2,
                              list_t **shape1_p,
                              list_t **shape2_p,
                              collision_handler_t handler,
                              void *aux,
                              free_func_t freer,
                              force_creator_t force_creator) {
    list_t *bodies = list_init(2, NULL);
    list_add(bodies, body1);
    list_add(bodies, body2);

    aux_collision_t *aux_c = aux_collision_init(2, handler, aux, freer);
    aux_add_body((aux_t *)aux_c, body1);
    aux_add_body((aux_t *)aux_c, body2);
    aux_c->shape1_p = shape1_p;
    aux_c->shape2_p = shape2_p;

    physics_add_force(physics,
                      (force_creator_t)force_creator,
                      aux_c,
                      bodies,
                      (free_func_t)aux_free);
}

// void create_boundary_collision(scene_t *scene,
//                                body_t * ball,
//                                body_t * wall,
//                                double elasticity,
//                                double radius,
//                                double buffer) {

//     aux_t *aux = aux_init(3, 0);
//     aux_add_constant(aux, elasticity);
//     aux_add_constant(aux, radius);
//     aux_add_constant(aux, buffer);

//     create_collision(scene,
//                      ball,
//                      wall,
//                      (collision_handler_t)collision_handler_boundary,
//                      aux,
//                      (free_func_t)aux_free);
// }

void create_destructive_collision(physics_t *physics,
                                  body_t *body1,
                                  body_t *body2) {
    create_collision(physics,
                     body1,
                     body2,
                     (collision_handler_t)collision_handler_destructive,
                     NULL,
                     NULL);
}

void create_hippo_ball_collision(physics_t *physics,
                                 body_t *ball,
                                 body_t *hippo) {
    create_asymmetric_destructive_collision(physics, ball, hippo);
}

void create_asymmetric_destructive_collision(physics_t *physics,
                                             body_t *body_to_remove,
                                             body_t *body_other) {
    create_collision(
        physics,
        body_to_remove,
        body_other,
        (collision_handler_t)collision_handler_asymmetric_destructive,
        NULL,
        NULL);
}

void create_hippo_collision(physics_t *physics,
                            double elasticity,
                            body_t *hippo1,
                            body_t *hippo2) {
    create_physics_collision(physics, elasticity, hippo1, hippo2);
}

void create_physics_collision(physics_t *physics,
                              double elasticity,
                              body_t *body1,
                              body_t *body2) {
    aux_t *aux = aux_init(1, 0);
    aux_add_constant(aux, elasticity);

    create_collision(physics,
                     body1,
                     body2,
                     (collision_handler_t)collision_handler_physics,
                     aux,
                     (free_func_t)aux_free);
}

void create_physics_collision_shapes(physics_t *physics,
                                     double elasticity,
                                     body_t *body1,
                                     body_t *body2,
                                     list_t **shape1_p,
                                     list_t **shape2_p) {
    aux_t *aux = aux_init(1, 0);
    aux_add_constant(aux, elasticity);

    create_collision_shapes(physics,
                            body1,
                            body2,
                            shape1_p,
                            shape2_p,
                            (collision_handler_t)collision_handler_physics,
                            aux,
                            (free_func_t)aux_free);
}

void create_physics_spin_collision(physics_t *physics,
                                   double elasticity,
                                   double friction,
                                   body_t *body1,
                                   body_t *body2) {
    aux_t *aux = aux_init(2, 0);
    aux_add_constant(aux, elasticity);
    aux_add_constant(aux, friction);

    create_collision(physics,
                     body1,
                     body2,
                     (collision_handler_t)collision_handler_physics_spin,
                     aux,
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

void mediate_collision(aux_collision_t *aux, list_t *shape1, list_t *shape2) {
    collision_info_t c_info = find_collision(shape1, shape2);

    if (c_info.collided == true) {
        body_t *body1 = (body_t *)list_get(aux->bodies, 0);
        body_t *body2 = (body_t *)list_get(aux->bodies, 1);
        if (!aux->already_collided) {
            aux->handler(body1,
                         body2,
                         c_info.axis,
                         c_info.collision_point,
                         aux->handler_aux);
        }
        aux->already_collided = true;
    } else {
        aux->already_collided = false;
    }
}

void force_creator_collision(aux_collision_t *aux) {
    body_t *body1 = (body_t *)list_get(aux->bodies, 0);
    body_t *body2 = (body_t *)list_get(aux->bodies, 1);
    list_t *shape1 = body_get_shape_nocp(body1);
    list_t *shape2 = body_get_shape_nocp(body2);
    mediate_collision(aux, shape1, shape2);
}

void force_creator_collision_shapes(aux_collision_t *aux) {
    list_t *shape1 = *(aux->shape1_p);
    list_t *shape2 = *(aux->shape2_p);
    mediate_collision(aux, shape1, shape2);
}

void force_creator_collision_shape(aux_collision_t *aux) {
    list_t *shape1 = *(aux->shape1_p);
    list_t *shape2 = body_get_shape_nocp((body_t *)list_get(aux->bodies, 1));
    mediate_collision(aux, shape1, shape2);
}

// void collision_handler_boundary(body_t *ball,
//                                 body_t *dontuse,
//                                 vector_t axis,
//                                 vector_t collision_point,
//                                 aux_t * aux) {
//     double ma = body_get_mass(ball);
//     double CR = *(double *)list_get(aux->constants, 0);
//     double ua = vec_dot(body_get_velocity(ball), axis);
//     double Jn = ma * (1 + CR) * (0 - ua);
//     body_add_impulse(ball, vec_multiply(Jn, axis));
// }

void collision_handler_destructive(body_t *body1,
                                   body_t *body2,
                                   vector_t axis,
                                   vector_t collision_point,
                                   aux_t *aux) {
    body_remove(body1);
    body_remove(body2);
}

void collision_handler_asymmetric_destructive(body_t *body_to_remove,
                                              body_t *body_other,
                                              vector_t axis,
                                              vector_t collision_point,
                                              aux_t *aux) {
    body_remove(body_to_remove);
    // DO NOT REMOVE OTHER BODY
}

void collision_handler_physics(body_t *body1,
                               body_t *body2,
                               vector_t axis,
                               vector_t collision_point,
                               aux_t *aux) {
    double ma = body_get_mass(body1);
    double mb = body_get_mass(body2);
    double CR = *(double *)list_get(aux->constants, 0);
    double ua = vec_dot(body_get_velocity(body1), axis);
    double ub = vec_dot(body_get_velocity(body2), axis);
    double r_mass;

    // check for INFINITE masses
    if (ma == INFINITY) {
        if (mb == INFINITY) {
            body_set_velocity(body1, VEC_ZERO);
            body_set_velocity(body2, VEC_ZERO);
            return;
        }
        r_mass = mb;
    } else if (mb == INFINITY) {
        r_mass = ma;
    } else {
        r_mass = ma * mb / (ma + mb);
    }
    double Jn = r_mass * (1 + CR) * (ub - ua); // 1800

    body_add_impulse(body1, vec_multiply(Jn, axis));
    body_add_impulse(body2, vec_multiply(-Jn, axis));
}

void collision_handler_physics_spin(body_t *body1,
                                    body_t *body2,
                                    vector_t axis,
                                    vector_t collision_point,
                                    aux_t *aux) {
    double ma = body_get_mass(body1);
    double mb = body_get_mass(body2);
    double CR = *(double *)list_get(aux->constants, 0);
    double f_c = *(double *)list_get(aux->constants, 1);
    double ua = vec_dot(body_get_velocity(body1), axis);
    double ub = vec_dot(body_get_velocity(body2), axis);
    double mass_divisor;

    // check for INFINITE masses
    if (ma == INFINITY) {
        if (mb == INFINITY) {
            body_set_velocity(body1, VEC_ZERO);
            body_set_velocity(body2, VEC_ZERO);
            return;
        }
        mass_divisor = (1 / mb);
    } else if (mb == INFINITY) {
        mass_divisor = (1 / ma);
    } else {
        mass_divisor = (1 / ma) + (1 / mb);
    }
    double Jn = (1 + CR) * (ub - ua) / mass_divisor;
    double impulse_friction = f_c * Jn;
    double angle_v_sum
        = (body_get_angular_velocity(body1) + body_get_angular_velocity(body2));
    vector_t impulse = vec_multiply(Jn - f_c * CR * angle_v_sum / 100, axis);
    vector_t impulse_new = vec_rotate(
        impulse,
        -angle_v_sum / 50); // rotate ccwise if angular velocity is negative

    body_set_angular_velocity(
        body1,
        body_get_angular_velocity(body1)
            + ((ma == INFINITY)
                   ? 0
                   : (f_c * CR * angle_v_sum + impulse_friction / 100)));
    body_set_angular_velocity(
        body2,
        body_get_angular_velocity(body2)
            - ((mb == INFINITY)
                   ? 0
                   : (f_c * CR * angle_v_sum + impulse_friction / 100)));

    body_add_impulse(body1, impulse_new);
    body_add_impulse(body2, vec_negate(impulse_new));
}
