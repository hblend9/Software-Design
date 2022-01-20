#include "polygon.h"
#include <math.h>

double polygon_area(list_t *polygon) {
    // Compute area using the Shoelace formula.
    size_t n = list_size(polygon);
    // Running sum.
    double sum = 0;
    for (size_t i = 0; i < n; i++) {
        sum += vec_cross(*(vector_t *)list_get(polygon, i),
                         *(vector_t *)list_get(polygon, (i + 1) % n));
    }
    // Shoelace formula: A(polygon) = |sum| / 2.
    return fabs(sum) / 2;
}

vector_t polygon_centroid(list_t *polygon) {
    // Compute using this formula:
    // https://en.wikipedia.org/wiki/Centroid#Of_a_polygon
    size_t n = list_size(polygon);
    double area = polygon_area(polygon);
    double sum_x = 0;
    double sum_y = 0;
    for (size_t i = 0; i < n; i++) {
        sum_x += (((vector_t *)list_get(polygon, i))->x
                  + ((vector_t *)list_get(polygon, (i + 1) % n))->x)
                 * (vec_cross(*(vector_t *)list_get(polygon, i),
                              *(vector_t *)list_get(polygon, (i + 1) % n)));
        sum_y += (((vector_t *)list_get(polygon, i))->y
                  + ((vector_t *)list_get(polygon, (i + 1) % n))->y)
                 * (vec_cross(*(vector_t *)list_get(polygon, i),
                              *(vector_t *)list_get(polygon, (i + 1) % n)));
    }
    vector_t centroid = {.x = sum_x / (6 * area), .y = sum_y / (6 * area)};
    return centroid;
}

void polygon_translate(list_t *polygon, vector_t translation) {
    for (size_t i = 0; i < list_size(polygon); i++) {
        vector_t *v = list_get(polygon, i);
        *v = vec_add(*v, translation);
    }
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
    for (size_t i = 0; i < list_size(polygon); i++) {
        vector_t *v = list_get(polygon, i);
        *v = vec_rotate_relative(*v, angle, point);
    }
}
