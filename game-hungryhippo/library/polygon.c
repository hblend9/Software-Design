#include "polygon.h"
#include "sdl_wrapper.h"
#include <math.h>
#include <stdlib.h>

/*** PRIVATE PROTOTYPES ***/

/*** DEFINITIONS ***/

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

void polygon_set_centroid(list_t *polygon, vector_t centroid) {
    polygon_translate(polygon,
                      vec_subtract(centroid, polygon_centroid(polygon)));
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
    for (size_t i = 0; i < list_size(polygon); i++) {
        vector_t *v = list_get(polygon, i);
        *v = vec_rotate_relative(*v, angle, point);
    }
}

list_t *polygon_init_from_path(const char *path) {
    return list_init_from_path(path, free, (parse_record_func_t)vec_parse_str);
}

list_t *polygon_init_many_from_paths(const list_t *paths) {
    list_t *shapes = list_init(list_size(paths), free);
    for (size_t i = 0; i < list_size(paths); i++) {
        list_add(shapes, polygon_init_from_path(list_get(paths, i)));
    }
    return shapes;
}

void polygon_scale(list_t *polygon, double factor) {
    if (factor <= 0) {
        fprintf(stderr,
                "Fatal error: negative polygon_scale factor invalid.\n");
    }

    vector_t old_centroid = polygon_centroid(polygon);

    for (size_t i = 0; i < list_size(polygon); i++) {
        vector_t *v = list_get(polygon, i);
        *v = vec_multiply(factor, *v);
    }

    // translate back to original centroid
    polygon_set_centroid(polygon, old_centroid);
}

void polygon_scr_to_sce(list_t *polygon) {
    // Apply screen-to-sceen scaling factor.
    polygon_scale(polygon, 1.0 / sdl_sce_to_scr_scale());
    // Reflect vertically since screen y-coords are opposite screen y-coords.
    vector_t c = polygon_centroid(polygon);
    for (size_t i = 0; i < list_size(polygon); i++) {
        vector_t *v = list_get(polygon, i);
        *v = (vector_t){v->x, c.y - (v->y - c.y)};
    }
    // We need to reverse order after reflection to preserve anticlockwise
    // orientation.
    list_reverse(polygon);
}

vector_t polygon_topleft(list_t *polygon) {
    assert(list_size(polygon) > 3);

    vector_t topleft = *(vector_t *)list_get(polygon, 0);
    for (size_t i = 0; i < list_size(polygon); i++) {
        vector_t *v = list_get(polygon, i);
        if (topleft.x > v->x) {
            topleft.x = v->x;
        }
        if (topleft.y < v->y) {
            topleft.y = v->y;
        }
    }
    return topleft;
}

vector_t polygon_botright(list_t *polygon) {
    assert(list_size(polygon) > 3);

    vector_t botright = *(vector_t *)list_get(polygon, 0);
    for (size_t i = 0; i < list_size(polygon); i++) {
        vector_t *v = list_get(polygon, i);
        if (botright.x < v->x) {
            botright.x = v->x;
        }
        if (botright.y > v->y) {
            botright.y = v->y;
        }
    }
    return botright;
}

vector_t polygon_center(list_t *polygon) {
    return vec_multiply(
        1.0 / 2.0,
        vec_add(polygon_topleft(polygon), polygon_botright(polygon)));
}

vector_t polygon_centroid_to_topleft(list_t *polygon) {
    return vec_subtract(polygon_topleft(polygon), polygon_centroid(polygon));
}

vector_t polygon_centroid_to_center(list_t *polygon) {
    return vec_subtract(polygon_center(polygon), polygon_centroid(polygon));
}
