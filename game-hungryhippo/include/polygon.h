/**
 * Note that "polygon" and "shape" are synonymous, the former retained for
 * legacy support.
 */
#ifndef __POLYGON_H__
#define __POLYGON_H__

#include "list.h"
#include "vector.h"

/**
 * Computes the area of a polygon.
 * See https://en.wikipedia.org/wiki/Shoelace_formula#Statement.
 *
 * @param polygon the list of vertices that make up the polygon,
 * listed in a counterclockwise direction. There is an edge between
 * each pair of consecutive vertices, plus one between the first and last.
 * @return the area of the polygon
 */
double polygon_area(list_t *polygon);

/**
 * Computes the center of mass of a polygon.
 * See https://en.wikipedia.org/wiki/Centroid#Of_a_polygon.
 *
 * @param polygon the list of vertices that make up the polygon,
 * listed in a counterclockwise direction. There is an edge between
 * each pair of consecutive vertices, plus one between the first and last.
 * @return the centroid of the polygon
 */
vector_t polygon_centroid(list_t *polygon);

/**
 * Translates all vertices in a polygon by a given vector.
 * Note: mutates the original polygon.
 *
 * @param polygon the list of vertices that make up the polygon
 * @param translation the vector to add to each vertex's position
 */
void polygon_translate(list_t *polygon, vector_t translation);

void polygon_set_centroid(list_t *polygon, vector_t centroid);

/**
 * Rotates vertices in a polygon by a given angle about a given point.
 * Note: mutates the original polygon.
 *
 * @param polygon the list of vertices that make up the polygon
 * @param angle the angle to rotate the polygon, in radians.
 * A positive angle means counterclockwise.
 * @param point the point to rotate around
 */
void polygon_rotate(list_t *polygon, double angle, vector_t point);

/**
 * Initalize a shape from a string path.
 */
list_t *polygon_init_from_path(const char *path);

/**
 * Create a list of polygons from a list of paths.
 */
list_t *polygon_init_many_from_paths(const list_t *paths);

/**
 * Scale the polygon by factor relative to the origin (0, 0).
 */
void polygon_scale(list_t *polygon, double factor);

/**
 * (Almost) convert a polygon from screen to scene coordinates. We say "almost"
 * because, while this function scales and reflects the polygon correctly, it
 * does not attempt to map the centroid from screen to scene coordinates. We
 * recommend using 'polygon_set_centroid' to place the polygon after conversion.
 */
void polygon_scr_to_sce(list_t *polygon);

/**
 * Compute vector pointing from the centroid of the polygon to the top left
 * corner of its rectangle.
 */
vector_t polygon_centroid_to_topleft(list_t *polygon);

/**
 * Compute vector pointing from the rectangle's center to the centroid of the
 * polygon.
 */
vector_t polygon_centroid_to_center(list_t *polygon);

/**
 * Compute the center (not necessarily centroid aka center of mass) of the
 * polygon's rectangle.
 */
vector_t polygon_center(list_t *polygon);

/**
 * Compute the top left corner of the polygon's rectangle.
 */
vector_t polygon_topleft(list_t *polygon);

/**
 * Compute the bottom right corner of the polygon's rectangle.
 */
vector_t polygon_botright(list_t *polygon);

#endif // #ifndef __POLYGON_H__
