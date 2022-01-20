#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


const vector_t VEC_ZERO = {.x = 0, .y = 0};


vector_t vec_add(vector_t v1, vector_t v2) {
    double v3x = v1.x + v2.x;
    double v3y = v1.y + v2.y;
    vector_t v3 = {v3x, v3y};
    return v3;
}


vector_t vec_subtract(vector_t v1, vector_t v2) {
    vector_t v3 = vec_add(v1, vec_negate(v2));
    return v3;
}


vector_t vec_negate(vector_t v) {
    double v2x = -1 * v.x;
    double v2y = -1 * v.y;
    vector_t v2 = {v2x, v2y};
    return v2;
}


vector_t vec_multiply(double scalar, vector_t v){
    double v2x = v.x * scalar;
    double v2y = v.y * scalar;
    vector_t v2 = {v2x, v2y};
    return v2;
}


double vec_dot(vector_t v1, vector_t v2){
    return v1.x * v2.x + v1.y * v2.y;
}


double vec_cross(vector_t v1, vector_t v2){
    return v1.x * v2.y - v2.x * v1.y;
}


vector_t vec_rotate(vector_t v, double angle){
    double v2x = v.x * cos(angle) - v.y * sin(angle);
    double v2y = v.x * sin(angle) + v.y * cos(angle);
    vector_t v2 = {v2x, v2y};
    return v2;
}