#include "shapes_geometry.h"
#include "body.h"
#include "color.h"
#include "list.h"
#include "polygon.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

body_t *make_rectangle(rgb_color_t color,
                       vector_t initial_position,
                       double height,
                       double width,
                       double mass,
                       void *rec_info,
                       free_func_t freer) {
    list_t *rectangle_points
        = make_rectangle_polygon(initial_position, height, width);

    body_t *rectangle
        = body_init_with_info(rectangle_points, mass, color, rec_info, freer);

    return rectangle;
}

list_t *
make_rectangle_polygon(vector_t initial_position, double height, double width) {
    list_t *rectangle_points = list_init(4, free);

    vector_t *left = malloc(sizeof(vector_t));
    assert(left != NULL);
    left->x = -width / 2.0 + initial_position.x;
    left->y = -height / 2.0 + initial_position.y;
    list_add(rectangle_points, left);

    vector_t *right = malloc(sizeof(vector_t));
    assert(right != NULL);
    right->x = width / 2.0 + initial_position.x;
    right->y = -height / 2.0 + initial_position.y;
    list_add(rectangle_points, right);

    vector_t *top_r = malloc(sizeof(vector_t));
    assert(top_r != NULL);
    top_r->x = width / 2.0 + initial_position.x;
    top_r->y = height / 2.0 + initial_position.y;
    list_add(rectangle_points, top_r);

    vector_t *top_l = malloc(sizeof(vector_t));
    assert(top_l != NULL);
    top_l->x = -width / 2.0 + initial_position.x;
    top_l->y = height / 2.0 + initial_position.y;
    list_add(rectangle_points, top_l);

    return rectangle_points;
}

body_t *make_triangle(rgb_color_t color,
                      vector_t initial_position,
                      double size,
                      double mass,
                      void *tri_info,
                      free_func_t freer) {
    list_t *triangle_points = make_triangle_polygon(initial_position, size);

    body_t *triangle
        = body_init_with_info(triangle_points, mass, color, tri_info, freer);

    body_set_centroid(triangle, initial_position);

    return triangle;
}

list_t *make_triangle_polygon(vector_t initial_position, double size) {
    list_t *triangle_points = list_init(3, free);

    vector_t *left = malloc(sizeof(vector_t));
    assert(left != NULL);
    left->x = size / 2.0 + initial_position.x;
    left->y = initial_position.y;
    list_add(triangle_points, left);

    vector_t *right = malloc(sizeof(vector_t));
    assert(right != NULL);
    right->x = -size / 2.0 + initial_position.x;
    right->y = initial_position.y;
    list_add(triangle_points, right);

    vector_t *tip = malloc(sizeof(vector_t));
    assert(tip != NULL);
    tip->x = initial_position.x;
    tip->y = size / 2.0 + initial_position.y;
    list_add(triangle_points, tip);

    return triangle_points;
}

body_t *make_circle(double radius,
                    double resolution,
                    vector_t initial_position,
                    double mass,
                    void *circle_info,
                    free_func_t freer,
                    rgb_color_t color) {
    list_t *ball_points
        = make_circle_polygon(radius, resolution, initial_position);

    body_t *ball
        = body_init_with_info(ball_points, mass, color, circle_info, freer);
    body_set_centroid(ball, initial_position);

    return ball;
}

list_t *make_circle_polygon(double radius,
                            double resolution,
                            vector_t initial_position) {
    assert(radius >= 0);
    assert(resolution >= 1);

    list_t *ball_points = list_init(resolution, free);

    double angle = (2 * M_PI) / resolution;

    // initializes a body centered at (0,0)
    vector_t *p = NULL;
    for (size_t i = 0; i < resolution; i++) {
        p = malloc(sizeof(vector_t));
        assert(p != NULL);
        p->x = 0;
        p->y = radius;
        double curr_angle = i * angle;
        *p = vec_rotate(*p, curr_angle);
        p->x += initial_position.x;
        p->y += initial_position.y;
        list_add(ball_points, p);
    }

    return ball_points;
}
