#include <stdlib.h>
#include <stdio.h>
#include "body.h"
#include "list.h"
#include <math.h>

typedef struct body{
    list_t *shape;
    rgb_color_t color;
    vector_t centroid;

    vector_t velocity;

    double mass;
    double direction;
    double angle;
}body_t;

body_t *body_init(list_t *shape, double mass, rgb_color_t color){
    body_t *my_body = malloc(sizeof(body_t));

    my_body->shape = shape;
    my_body->color = color;
    my_body->centroid = polygon_centroid(shape);

    vector_t initial_velocity = {.x = 0, .y = 0};
    my_body->velocity = initial_velocity;

    my_body->mass = mass;
    my_body->angle = 0;

    return my_body;
}

void body_free(body_t *body){
    list_free(body->shape);
    free(body);
}

list_t *body_get_shape(body_t *body){
    list_t *shape_copy = list_init(list_size(body->shape), list_get_freer(body->shape));

    for (size_t i = 0; i < list_size(body->shape); i++){
        vector_t *prev = list_get(body->shape, i);
        vector_t *new = malloc(sizeof(vector_t));
        new->x = prev->x;
        new->y = prev->y;
        list_add(shape_copy, new);
    }

    return shape_copy;
}

vector_t body_get_centroid(body_t *body){
    return body->centroid;
}

vector_t body_get_velocity(body_t *body){
    return body->velocity;
}

rgb_color_t body_get_color(body_t *body){
    return body->color;
}

// double body_get_direction(body_t *body) {
//   return body->direction;
// }

void body_set_centroid(body_t *body, vector_t x){
    polygon_translate(body_get_shape(body), vec_subtract(x, body->centroid));
    body->centroid = x;
}

void body_set_velocity(body_t *body, vector_t v){
    body->velocity = v; // hey i'm going to bed so i'm going to push this version so that you can pull it
}

/**
 * Changes a body's orientation in the plane.
 * The body is rotated about its center of mass.
 * Note that the angle is *absolute*, not relative to the current orientation.
 *
 * @param body a pointer to a body returned from body_init()
 * @param angle the body's new angle in radians. Positive is counterclockwise.
 */
void body_set_rotation(body_t *body, double angle){
    polygon_rotate(body_get_shape(body), angle, body->centroid);
}

// void body_set_direction(body_t *body, double direction) {
//     double new_angle = direction - body->direction;
//     vector_t center = polygon_centroid(body_get_shape(body));
//     polygon_rotate(body_get_shape(body), new_angle, center);
//     body->direction = direction;
// }

// Moves body
void body_tick(body_t *body, double dt){
    vector_t translation = (body->velocity);
    vec_multiply(dt, translation);
    polygon_translate(body->shape, translation);

    // vector_t translation = {(body->velocity).x * dt, (body->velocity).y * dt};
    // polygon_translate(body_get_shape(body), translation);
    // vector_t center = polygon_centroid(body_get_shape(body));
    // polygon_rotate(body_get_shape(body), body->angle * dt, center);
    // body->direction = fmod((body->direction + body->angle * dt), 2 * M_PI);
    // double new_x = (body->acceleration).x * dt + (body->velocity).x;
    // double new_y = (body->acceleration).y * dt + (body->velocity).y;
    // vector_t new_velocity = {new_x, new_y};
    // body->velocity = new_velocity;
}