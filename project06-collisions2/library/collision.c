#include "collision.h"
#include "list.h"
#include "polygon.h"
#include "vector.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

const collision_info_t NO_COLLISION_INFO = {.collided = false, .axis = {0, 0}};

// Private Functions Prototypes

// returns the amount of overlap of the projection of two shapes onto the
// normalized perpendicular line of the edge
double get_overlap_by_axis(vector_t line, list_t *shape1, list_t *shape2);

// return the amount of overlap two sets have
// if <= 0, no overlap
double get_overlap(vector_t min_max1, vector_t min_max2);

// finds min and max from list_t shape given a line to project the vertices on
// to line is normalized returns a "number-line" vector_t with {.x = min, .y =
// max} a helper function to find_if_overlap_by_edge
vector_t find_shape_projection(vector_t line, list_t *shape);

// Function Definitions

collision_info_t find_collision(list_t *shape1, list_t *shape2) {
    size_t shape1_size = list_size(shape1);
    size_t shape2_size = list_size(shape2);
    vector_t centroid_1 = polygon_centroid(shape1);
    vector_t centroid_2 = polygon_centroid(shape2);
    double curr_overlap = 0;
    double min_overlap = INFINITY;
    vector_t min_overlap_axis;
    vector_t curr_axis;
    vector_t collision_point;
    vector_t p1;
    vector_t p2;
    double d1;
    double d2;

    // loop over each edge of both polygons
    for (size_t i = 0; i < shape1_size; i++) {
        p1 = *(vector_t *)list_get(shape1, i);
        p2 = *(vector_t *)list_get(shape1, (i + 1) % shape1_size);

        // points out from body 1
        curr_axis = find_norm_perpendicular_vec(vec_subtract(p1, p2));
        curr_overlap = get_overlap_by_axis(curr_axis, shape1, shape2);
        if (curr_overlap <= 0) {
            // exit immediately if no collision occured
            return NO_COLLISION_INFO;
        }
        if (curr_overlap < min_overlap) {
            d1 = vec_magnitude(vec_subtract(p1, centroid_2));
            d2 = vec_magnitude(vec_subtract(p2, centroid_2));
            collision_point = d1 < d2 ? p1 : p2;
            min_overlap = curr_overlap;
            min_overlap_axis = curr_axis;
        }
    }
    for (size_t m = 0; m < shape2_size; m++) {
        p1 = *(vector_t *)list_get(shape2, m);
        p2 = *(vector_t *)list_get(shape2, (m + 1) % shape2_size);
        // points in to body 2
        curr_axis = find_norm_perpendicular_vec(vec_subtract(p2, p1));
        curr_overlap = get_overlap_by_axis(curr_axis, shape1, shape2);
        if (curr_overlap <= 0) {
            // exit immediately if no collision occured
            return NO_COLLISION_INFO;
        }
        if (curr_overlap < min_overlap) {
            d1 = vec_magnitude(vec_subtract(p1, centroid_1));
            d2 = vec_magnitude(vec_subtract(p2, centroid_1));
            collision_point = d1 < d2 ? p1 : p2;
            min_overlap = curr_overlap;
            min_overlap_axis = curr_axis;
        }
    }
    // all edges' projections overlap, return minimum overlap axis
    return (collision_info_t){.collided = true,
                              .axis = min_overlap_axis,
                              .collision_point = collision_point};
}

double get_overlap_by_axis(vector_t line, list_t *shape1, list_t *shape2) {
    vector_t min_max1 = find_shape_projection(line, shape1);
    vector_t min_max2 = find_shape_projection(line, shape2);
    return get_overlap(min_max1, min_max2);
}

double get_overlap(vector_t min_max1, vector_t min_max2) {
    double min1 = min_max1.x;
    double max1 = min_max1.y;
    double min2 = min_max2.x;
    double max2 = min_max2.y;
    return fmin(max1, max2) - fmax(min1, min2);
}

vector_t find_shape_projection(vector_t line, list_t *shape) {
    double min = INFINITY;
    double max = -INFINITY;
    for (size_t i = 0; i < list_size(shape); i++) {
        double curr_project = vec_dot(line, *(vector_t *)list_get(shape, i));
        if (curr_project > max) {
            max = curr_project;
        }
        if (curr_project < min) {
            min = curr_project;
        }
    }
    return (vector_t){.x = min, .y = max};
}

vector_t find_norm_perpendicular_vec(vector_t original_vec) {
    double length = vec_magnitude(original_vec);
    vector_t norm_original = vec_multiply((1.0 / length), original_vec);
    vector_t norm_perp = {.x = -(norm_original.y), .y = (norm_original.x)};
    return norm_perp;
}
