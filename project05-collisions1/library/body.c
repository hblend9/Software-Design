#include "body.h"
#include "list.h"
#include "polygon.h"
#include "vector.h"
#include <assert.h>
#include <stdlib.h>
#include "test_util.h"

typedef struct body {
    list_t *shape; // Polygon.

    rgb_color_t color;

    vector_t centroid;
    vector_t velocity;
    double mass;  // Nonnegative.
    double angle; // Absolute, where 0 rad is initial shape's orientation.
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

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
    body_t *b = malloc(sizeof(body_t));
    assert(b != NULL);

    body_set_mass(b, mass);
    b->shape = shape;
    body_set_color(b, color);
    body_set_velocity(b, VEC_ZERO);
    b->centroid = polygon_centroid(shape);
    b->angle = 0;
    /* DEPRECATED: body_set_angular_dynamics(b, 0, 0, 0); */
    b->force = VEC_ZERO;
    b->impulse = VEC_ZERO;
    b->removed = false;
    b->info = NULL;
    b->info_freer = NULL;
    b->acceleration = VEC_ZERO;

    return b;
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
    list_free(body->shape);
    if (body->info_freer != NULL) {
        body->info_freer(body->info);
    }
    free(body);
}

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

list_t *body_get_shape(body_t *body) {
    return list_copy(body->shape, (copy_func_t)vec_p_copy);
}

vector_t body_get_centroid(body_t *body) {
    return body->centroid;
}

vector_t body_get_velocity(body_t *body) {
    return body->velocity;
}

double body_get_rotation(body_t *body) {
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

// DEPRECATED, possible source of precision error bug.
void body_set_angular_dynamics(body_t *body,
                               double angle,
                               double angular_velocity,
                               double angular_acceleration) {
    body_set_rotation(body, angle);
    body_set_angular_velocity(body, angular_velocity);
    body_set_angular_acceleration(body, angular_acceleration);
}

void body_set_centroid(body_t *body, vector_t x) {
    polygon_translate(body->shape, vec_subtract(x, body_get_centroid(body)));
    body->centroid = x;
}

void body_translate(body_t *body, vector_t translation) {
    polygon_translate(body->shape, translation);
    body->centroid = vec_add(body_get_centroid(body), translation);
}

void body_set_velocity(body_t *body, vector_t v) {
    body->velocity = v;
}

void body_set_rotation(body_t *body, double angle) {
    if (within(1e-7, angle, 0)) {
        return;
    }
    polygon_rotate(body->shape,
                   angle - body_get_rotation(body),
                   body_get_centroid(body));
    body->angle = angle;
}

void body_rotate(body_t *body, double angle) {
    if (within(1e-7, angle, 0)) {
        return;
    }
    polygon_rotate(body->shape, angle, body_get_centroid(body));
    body->angle = body_get_rotation(body) + angle;
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
    // TODO
    /*    /1*, body_get_acceleration(body)); // Deprecated acceleration. *1/ */

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
