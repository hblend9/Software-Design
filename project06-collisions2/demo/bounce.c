#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// updated to use lists of vectors instead of vec-lists and color_rgb_t instead
// of float[3]

// these define the screen size
#define MIN_X 0
#define MIN_Y 0
#define MAX_X 1000
#define MAX_Y 500
// velocity vector components
#define VEL_X 200
#define VEL_Y 200
// angular velocity in radians
#define VEL_ANG 0.5

// returns a list_t polygon of vectors that represents a star with points number
// of outer points of a given size (outer_r) and thickness (inner_r)
list_t *star(size_t points, double outer_r, double inner_r) {
    assert(points > 2);
    assert(outer_r > 0 && inner_r > 0);

    // star has double the points of outer points
    points = 2 * points;
    double angle = 2 * M_PI / points;

    list_t *star = list_init(points, free);

    size_t i = 0;
    vector_t *p = NULL;
    for (i = 0; i < points; i++) {
        p = (vector_t *)malloc(sizeof(vector_t));
        assert(p != NULL);
        p->x = 0;
        p->y = ((i % 2 == 0) ? outer_r : inner_r);
        double alpha = i * angle;
        *p = vec_rotate(*p, alpha);
        list_add(star, p);
    }

    return star;
}

// checks if any point of the polygon object is out of bounds (has hit a wall)
// and adjusts its position and velocity assuming perfectly perfectly elastic
// collisions
bool collision(vector_t min, vector_t max, vector_t *velocity, list_t *poly) {
    size_t i;
    size_t n = list_size(poly);
    vector_t *p;

    for (i = 0; i < n; i++) {
        p = list_get(poly, i);
        if (p->x <= min.x) {
            // reset the position if it goes beyond boundaries (when dt is too
            // large)
            double diff = fabs(min.x - p->x);
            polygon_translate(poly, (vector_t){diff, 0});

            // do not switch direction if it was already switched
            // (can occur if a point rotates into the wall close to a previous
            // collision)
            if (velocity->x < 0) {
                velocity->x *= -1;
                return true;
            }
        }
        if (p->x >= max.x) {
            double diff = fabs(max.x - p->x);
            polygon_translate(poly, (vector_t){-diff, 0});
            if (velocity->x > 0) {
                velocity->x *= -1;
                return true;
            }
        }
        if (p->y <= min.y) {
            double diff = fabs(min.y - p->y);
            polygon_translate(poly, (vector_t){diff, 0});
            if (velocity->y < 0) {
                velocity->y *= -1;
                return true;
            }
        }
        if (p->y >= max.y) {
            double diff = fabs(max.y - p->y);
            polygon_translate(poly, (vector_t){-diff, 0});
            if (velocity->y > 0) {
                velocity->y *= -1;
                return true;
            }
        }
    }

    return false;
}

// updates the color of the object as it moves across the screen
void update_color(rgb_color_t *color,
                  vector_t position,
                  vector_t min,
                  vector_t max,
                  bool did_collide) {
    color->r = position.x / (max.x - min.x);
    color->g = position.y / (max.y - min.y);
    if (did_collide) {
        color->b = !color->b;
    }
}

// starts the animation of a star that collides with the boundaries of the drawn
// screen
int main(int argc, char *argv[]) {
    vector_t min = {MIN_X, MIN_Y};
    vector_t max = {MAX_X, MAX_Y};

    vector_t *velocity = malloc(sizeof(vector_t));
    assert(velocity != NULL);

    velocity->x = VEL_X;
    velocity->y = VEL_Y;

    sdl_init(min, max);

    list_t *poly = star(5, 50, 25);

    // starts the object centered at the center of the drawn window
    polygon_translate(poly, (vector_t){MAX_X / 2, MAX_Y / 2});

    double dt = 0;
    bool did_collide;
    vector_t c;

    // represents red, green, and blue, that can be updated if desired as object
    // moves
    rgb_color_t *color = malloc(sizeof(rgb_color_t));
    assert(color != NULL);
    color->r = 0;
    color->g = 1;
    color->b = 0;

    while (!sdl_is_done()) {
        dt = time_since_last_tick();

        c = polygon_centroid(poly);

        polygon_translate(poly, vec_multiply(dt, *velocity));
        polygon_rotate(poly, dt * VEL_ANG, c);
        did_collide = collision(min, max, velocity, poly);

        sdl_clear();
        update_color(color, c, min, max, did_collide);
        sdl_draw_polygon(poly, *color);
        sdl_show();
    }

    free(color);
    color = NULL;
    free(velocity);
    velocity = NULL;
    list_free(poly);
    poly = NULL;
}
