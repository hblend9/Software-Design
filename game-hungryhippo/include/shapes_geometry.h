#ifndef __SHAPES_GEOMETRY_H__
#define __SHAPES_GEOMETRY_H__

#include "body.h"
#include "color.h"

/**
 * returns a rectangle of given height, width, color, mass, and position
 * centered at the initial_position
 * @param color the color that the rectangle should be
 * @param initial_position the vector position the rectangle should be centered
 * at
 * @param height the height of the rectangle
 * @param weidth the width of the rectangle
 * @param mass the mass of the rectangle
 * @return new rectangle body_t*
 */
body_t *make_rectangle(rgb_color_t color,
                       vector_t initial_position,
                       double height,
                       double width,
                       double mass,
                       void *rec_info,
                       free_func_t freer);

/**
 * returns a list of points forming a rectangle of given height, width, and
 * initial_position
 * @param initial_position the vector position the rectangle should be centered
 * at
 * @param height the height of the rectangle
 * @param weidth the width of the rectangle
 */
list_t *
make_rectangle_polygon(vector_t initial_position, double height, double width);

/**
 * make_triangle returns a new body_t isoceles triangle at given initial
 * position
 * @param color the color that the triangle should be
 * @param initial_position the vector position the triangle should be centered
 * at
 * @param size the base of the triangle and half the height of the triangle
 * @param mass the mass of the triangle
 * @return new triangle body_t*
 */
body_t *make_triangle(rgb_color_t color,
                      vector_t initial_position,
                      double size,
                      double mass,
                      void *tri_info,
                      free_func_t freer);

/**
 *
 * make_triangle_polygon returns the list of points that make up the triangle
 * at the given initial position
 * @param initial_position the vector position the triangle should be centered
 * at
 * @param size the base of the triangle and half the height of the triangle
 */
list_t *make_triangle_polygon(vector_t initial_position, double size);

/**
 * make_circle returns a new body_t circle at given initial position
 * @param radius the radius of the circle
 * @param resolution the number of points to be made that are associated with
 * the circle
 * @param initial_position the vector position that the centroid should be
 * located
 * @param mass the mass of the circle
 * @param circle_info the void * info for this circle
 * @param freer the freer function for the info
 * @param color the color the circle should be
 */
body_t *make_circle(double radius,
                    double resolution,
                    vector_t initial_position,
                    double mass,
                    void *circle_info,
                    free_func_t freer,
                    rgb_color_t color);

/**
 * make circle polygon returns a list of points that make up the new circle
 * at the given initial position
 * @param radius the radius of the circle
 * @param resolution the number of points to be made that are associated with
 * the circle
 * @param initial_position the vector position that the centroid should be
 * located
 */
list_t *make_circle_polygon(double radius,
                            double resolution,
                            vector_t initial_position);

#endif // #ifndef __SHAPES_GEOMETRY_H__
