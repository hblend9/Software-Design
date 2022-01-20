#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

const vector_t VEC_ZERO = {.x = 0, .y = 0};
const vector_t E1 = {.x = 1, .y = 0};
const vector_t E2 = {.x = 0, .y = 1};
const vector_t E3 = {.x = 1, .y = 1};

const double _VEC_ANGLE_THRESHOLD = 1e-3;

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

vector_t vec_normalize(vector_t v) {
    return vec_multiply(1 / vec_magnitude(v), v);
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

double vec_angle_between(vector_t v1, vector_t v2, vector_t point_around) {
    assert(vec_magnitude(v1) > 0 && vec_magnitude(v2) > 0);
    v1 = vec_subtract(v1, point_around);
    v2 = vec_subtract(v2, point_around);
    double cos_angle
        = vec_dot(v1, v2) / (vec_magnitude(v1) * vec_magnitude(v2));
    assert(cos_angle != NAN && cos_angle != INFINITY);
    if (fabs(cos_angle) - 1 > 0 && fabs(cos_angle) - 1 < _VEC_ANGLE_THRESHOLD) {
        if (cos_angle > 1) {
            cos_angle = 1;
        } else if (cos_angle < 1) {
            cos_angle = -1;
        } else {
            assert(false && "Invalid state.");
        }
    }
    assert(cos_angle >= -1 && cos_angle <= 1);
    double l = acos(cos_angle);
    assert(l != NAN);
    if (v1.x > v2.x) {
        return -l;
    } else {
        return l;
    }
}

vector_t *vec_parse_str(const char *str) {
    vector_t *v = malloc(sizeof(vector_t));
    char *_str = malloc(strlen(str) + 1);
    strcpy(_str, str);

    char *x_str = strtok(_str, ",");
    char *y_str = strtok(NULL, ",");

    v->x = strtod(x_str, NULL);
    v->y = strtod(y_str, NULL);

    free(_str);

    return v;
}
