#include "list.h"
#include "vector.h"
#include <math.h>
#include <stdbool.h>

/**
 * Determines whether two convex polygons intersect.
 * The polygons are given as lists of vertices in counterclockwise order.
 * There is an edge between each pair of consecutive vertices,
 * and one between the first vertex and the last vertex.
 *
 * @param shape1 the first shape
 * @param shape2 the second shape
 * @return whether the shapes are colliding
 */

// Private Functions Prototypes

// returns whether the projection of two shapes onto the normalized
// perpendicular line of the edge overlaps
bool find_if_overlap_by_edge(vector_t edge, list_t *shape1, list_t *shape2);

// return whether two sets have a non-empty overlap
// a helper function to find_if_overlap_by_edge
bool find_if_overlap(vector_t min_max1, vector_t min_max2);

// finds min and max from list_t shape given a line to project the vertices on
// to line is normalized returns a "number-line" vector_t with {.x = min, .y =
// max} a helper function to find_if_overlap_by_edge
vector_t find_shape_projection(vector_t line, list_t *shape);

// returns a normalized vector perpendicular to the given vector
vector_t find_norm_perpendicular_vec(vector_t original_vec);

// Function Definitions

bool find_collision(list_t *shape1, list_t *shape2) {
    size_t shape1_size = list_size(shape1);
    size_t shape2_size = list_size(shape2);
    // loop over each edge of both polygons
    for (size_t i = 0; i < shape1_size; i++) {
        vector_t curr_edge = vec_subtract(
            *(vector_t *)list_get(shape1, i),
            *(vector_t *)list_get(shape1, (i + 1) % shape1_size));
        if (!find_if_overlap_by_edge(curr_edge, shape1, shape2)) {
            // exit immediately if no collision occured
            return false;
        }
    }
    for (size_t m = 0; m < shape2_size; m++) {
        vector_t curr_edge = vec_subtract(
            *(vector_t *)list_get(shape2, m),
            *(vector_t *)list_get(shape2, (m + 1) % shape2_size));
        if (!find_if_overlap_by_edge(curr_edge, shape1, shape2)) {
            // exit immediately if no collision occured
            return false;
        }
    }
    // if make it through loop, return true (all edges' projections overlap)
    return true;
}

bool find_if_overlap_by_edge(vector_t edge, list_t *shape1, list_t *shape2) {
    vector_t line = find_norm_perpendicular_vec(edge);
    vector_t min_max1 = find_shape_projection(line, shape1);
    vector_t min_max2 = find_shape_projection(line, shape2);
    return find_if_overlap(min_max1, min_max2);
}

bool find_if_overlap(vector_t min_max1, vector_t min_max2) {
    double min1 = min_max1.x;
    double max1 = min_max1.y;
    double min2 = min_max2.x;
    double max2 = min_max2.y;
    if (max1 > max2) {
        return (min1 < max2);
    } else if (max2 > max1) {
        return (min2 < max1);
    } else { // they are starting at the same position
        return true;
    }
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
