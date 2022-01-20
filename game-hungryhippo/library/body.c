#include "body.h"
#include "gfx_aux.h"
#include "list.h"
#include "polygon.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const double ANGLE_PRECISION = 1e-7;

typedef struct body {
    char *sprite_path;
    char *shape_path;

    gfx_aux_t *gfx_aux;
    list_t *shapes;
    list_t **shape_main;

    rgb_color_t color;

    vector_t *centroid;
    vector_t velocity;
    double mass; // Nonnegative.
    double moment_of_inertia;
    double *angle; // Absolute, where 0 rad is initial shape's orientation.
    double angular_velocity;
    double angular_acceleration;

    vector_t acceleration; // Deprecated.

    vector_t force;
    vector_t impulse;

    void *info;
    free_func_t info_freer;

    bool removed;
} body_t;

/**
 * Private protypes.
 */

void body_set_mass(body_t *body, double mass);

/**
 * Set the body's mass to a nonnegative double.
 */
void body_set_mass(body_t *body, double mass) {
    body->mass = mass;
}

/**
 * Definitions of public functions.
 */

// Primary init, others should be wrappers of this one.
body_t *body_init_with_gfx(double mass,
                           void *info,
                           free_func_t info_freer,
                           list_t *shapes,
                           gfx_aux_t *gfx_aux) {
    body_t *body = malloc(sizeof(body_t));
    assert(body != NULL);

    body_set_mass(body, mass);
    body_set_inertia(body, 0);

    if (shapes == NULL || list_size(shapes) < 1) {
        fprintf(
            stderr,
            "Fatal error: body must be initialized with at least one shape.\n");
        exit(1);
    }
    body->shapes = shapes;
    body->shape_main = malloc(sizeof(list_t *));
    assert(body->shape_main != NULL);
    *(body->shape_main) = list_get(shapes, 0);

    body_set_color(body, (rgb_color_t){0, 0, 0});
    body_set_velocity(body, VEC_ZERO);
    body->centroid = malloc(sizeof(vector_t));
    assert(body->centroid != NULL);
    *(body->centroid) = polygon_centroid(*(body->shape_main));
    body->angle = malloc(sizeof(double));
    assert(body->angle != NULL);
    *(body->angle) = 0;
    body->moment_of_inertia = INFINITY;
    body_set_angular_dynamics(body, 0, 0, 0);
    body->force = VEC_ZERO;
    body->impulse = VEC_ZERO;
    body->removed = false;
    body->info = info;
    body->info_freer = info_freer;
    body->acceleration = VEC_ZERO;

    body->gfx_aux = gfx_aux;
    if (body->gfx_aux != NULL) {
        gfx_aux_setup_with_body(body->gfx_aux, body);
    }

    return body;
}

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
    list_t *shapes = list_init(1, (free_func_t)list_free);
    list_add(shapes, shape);
    body_t *body = body_init_with_gfx(mass, NULL, NULL, shapes, NULL);
    body_set_color(body, color);
    return body;
}

body_t *body_init_with_info(list_t *shape,
                            double mass,
                            rgb_color_t color,
                            void *info,
                            free_func_t info_freer) {
    body_t *body = body_init(shape, mass, color);
    body->info = info;
    body->info_freer = info_freer;
    return body;
}

void body_free(body_t *body) {
    free(body->centroid);
    free(body->angle);
    list_free(body->shapes);
    if (body->gfx_aux != NULL) {
        gfx_aux_free(body->gfx_aux);
    }
    free(body->shape_main);
    if (body->info_freer != NULL && body->info != NULL) {
        body->info_freer(body->info);
    }
    free(body);
}

// Deprecated
body_t *body_copy(body_t *original) {
    list_t *shape_copy = body_get_shape(original);
    body_t *body_copy_obj = body_init(shape_copy,
                                      body_get_mass(original),
                                      body_get_color(original));
    body_set_centroid(body_copy_obj, body_get_centroid(original));
    body_set_velocity(body_copy_obj, body_get_velocity(original));
    body_set_angular_dynamics(body_copy_obj,
                              body_get_rotation(original),
                              body_get_angular_velocity(original),
                              body_get_angular_acceleration(original));
    return body_copy_obj;
}

double body_get_mass(body_t *body) {
    return body->mass;
}

void body_set_inertia(body_t *body, double inertia) {
    body->moment_of_inertia = inertia;
}

double body_get_inertia(body_t *body) {
    return body->moment_of_inertia;
}

list_t *body_get_shape(body_t *body) {
    return list_copy(body_get_shape_main(body), (copy_func_t)vec_p_copy);
}

list_t *body_get_shape_nocp(body_t *body) {
    return *(body->shape_main);
}

list_t *body_get_shape_main(body_t *body) {
    return *(body->shape_main);
}

list_t **body_get_shape_main_p(body_t *body) {
    return body->shape_main;
}

list_t *body_get_shape_alt(body_t *body, size_t idx) {
    if (idx < 0 || idx >= list_size(body->shapes)) {
        fprintf(stderr, "Fatal error: that shape doesn't exist.\n");
        exit(1);
    }
    return list_get(body->shapes, idx);
}

size_t body_get_num_shapes(body_t *body) {
    return list_size(body->shapes);
}

void body_set_shape_main(body_t *body, size_t idx) {
    *(body->shape_main) = body_get_shape_alt(body, idx);
}

vector_t body_get_centroid(body_t *body) {
    return *(body->centroid);
}

vector_t *body_get_anchor(body_t *body) {
    return body->centroid;
}

vector_t body_get_velocity(body_t *body) {
    return body->velocity;
}

double body_get_rotation(body_t *body) {
    return *(body->angle);
}

double *body_get_angle_p(body_t *body) {
    return body->angle;
}

double body_get_angular_velocity(body_t *body) {
    return body->angular_velocity;
}

double body_get_angular_acceleration(body_t *body) {
    return body->angular_acceleration;
}

rgb_color_t body_get_color(body_t *body) {
    return body->color;
}

void body_set_angular_dynamics(body_t *body,
                               double angle,
                               double angular_velocity,
                               double angular_acceleration) {
    body_set_rotation(body, angle);
    body_set_angular_velocity(body, angular_velocity);
    body_set_angular_acceleration(body, angular_acceleration);
}

void body_set_centroid(body_t *body, vector_t x) {
    for (size_t i = 0; i < list_size(body->shapes); i++) {
        list_t *shape = list_get(body->shapes, i);
        polygon_translate(shape, vec_subtract(x, *(body->centroid)));
    }
    *(body->centroid) = x;
}

void body_translate(body_t *body, vector_t translation) {
    for (size_t i = 0; i < list_size(body->shapes); i++) {
        list_t *shape = list_get(body->shapes, i);
        polygon_translate(shape, translation);
    }
    *(body->centroid) = vec_add(body_get_centroid(body), translation);
}

void body_set_velocity(body_t *body, vector_t v) {
    body->velocity = v;
}

void body_set_rotation(body_t *body, double angle) {
    if (fabs(angle) < ANGLE_PRECISION) {
        angle = 0;
    }
    for (size_t i = 0; i < list_size(body->shapes); i++) {
        list_t *shape = list_get(body->shapes, i);
        polygon_rotate(shape,
                       angle - body_get_rotation(body),
                       body_get_centroid(body));
    }
    *(body->angle) = angle;
}

void body_rotate(body_t *body, double angle) {
    body_set_rotation(body, body_get_rotation(body) + angle);
}

void body_set_angular_velocity(body_t *body, double value) {
    body->angular_velocity = value;
}

void body_set_angular_acceleration(body_t *body, double value) {
    body->angular_acceleration = value;
}

void body_set_color(body_t *body, rgb_color_t value) {
    body->color = value;
}

void body_add_force(body_t *body, vector_t force) {
    body->force = vec_add(body->force, force);
}

void body_add_impulse(body_t *body, vector_t impulse) {
    body->impulse = vec_add(body->impulse, impulse);
}

void body_tick(body_t *body, double dt) {
    // Linear dynamics.
    vector_t v1 = body_get_velocity(body);
    /*  body_get_acceleration(body)); // Deprecated acceleration. */

    body_add_impulse(body, vec_multiply(dt, body->force));
    vector_t v2 = vec_add(v1, vec_multiply(1.0 / body->mass, body->impulse));

    // Translate at the *average* of the velocities before and after the tick.
    vector_t dx = vec_multiply(dt / 2.0, vec_add(v1, v2));
    body_translate(body, dx);

    // The body may have a new velocity after each tick.
    body_set_velocity(body, v2);

    // Angular dynamics.
    double omega = body_get_angular_velocity(body);
    double alpha = body_get_angular_acceleration(body);
    // Second order: dtheta = omega dt + 1/2 alpha dt^2
    double dtheta = omega * dt + alpha * dt * dt / 2.0;
    double domega = alpha * dt;
    body_rotate(body, dtheta);
    // First order: domega = alpha dt
    body_set_angular_velocity(body, omega + domega);

    // Reset the forces and impulses accumulated on the body.
    body->force = VEC_ZERO;
    body->impulse = VEC_ZERO;
}

void body_remove(body_t *body) {
    body->removed = true;
}

bool body_is_removed(body_t *body) {
    return body->removed;
}

// Deprecated.
vector_t body_get_acceleration(body_t *body) {
    return body->acceleration;
}

// Deprecated.
void body_set_acceleration(body_t *body, vector_t value) {
    body->acceleration = value;
}

void *body_get_info(body_t *body) {
    return body->info;
}

void body_set_info(body_t *body, void *new_info, free_func_t freer) {
    body->info = new_info;
    body->info_freer = freer;
}

gfx_aux_t *body_get_gfx(body_t *body) {
    return body->gfx_aux;
}
