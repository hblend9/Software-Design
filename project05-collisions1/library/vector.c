#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

const vector_t VEC_ZERO = {.x = 0, .y = 0};

vector_t vec_add(vector_t v1, vector_t v2) {
    vector_t result = {.x = v1.x + v2.x, .y = v1.y + v2.y};
    return result;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
    vector_t result = {.x = v1.x - v2.x, .y = v1.y - v2.y};
    return result;
}

vector_t vec_negate(vector_t v) {
    vector_t result = {.x = -v.x, .y = -v.y};
    return result;
}

vector_t vec_multiply(double scalar, vector_t v) {
    vector_t result = {.x = scalar * v.x, .y = scalar * v.y};
    return result;
}

double vec_dot(vector_t v1, vector_t v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

double vec_cross(vector_t v1, vector_t v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

double vec_magnitude(vector_t v) {
    return sqrt(vec_dot(v, v));
}

vector_t vec_rotate(vector_t v, double angle) {
    vector_t result = {.x = v.x * cos(angle) - v.y * sin(angle),
                       .y = v.x * sin(angle) + v.y * cos(angle)};
    return result;
}

vector_t vec_rotate_relative(vector_t v, double angle, vector_t point) {
    return vec_add(point, vec_rotate(vec_subtract(v, point), angle));
}

vector_t *vec_p_copy(vector_t *v) {
    vector_t *vec_copy_obj = malloc(sizeof(vector_t));
    vec_copy_obj->x = v->x;
    vec_copy_obj->y = v->y;
    return vec_copy_obj;
}
