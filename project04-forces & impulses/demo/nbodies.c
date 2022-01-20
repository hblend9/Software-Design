/**
 * DEMO : Newtonian gravity
 *   no shapes ever get added after initialization
 *   no collisions between objects
 *   shapes go off screen and do not wrap back
 */

#include "body.h"
#include "color.h"
#include "forces.h"
#include "list.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_X 2000
#define MAX_Y 1000
const vector_t SCENE_DIMS = {.x = MAX_X, .y = MAX_Y};
#define OUTER_R 40
#define INNER_R 20
#define TOTAL_COLORS 14
const rgb_color_t COLORS[TOTAL_COLORS] = {
    {1.00, 0.00, 0.00}, // red
    {0.98, 0.23, 0.00}, // red orange
    {0.96, 0.60, 0.26}, // orange
    {0.98, 0.66, 0.00}, // gold
    {0.96, 0.96, 0.26}, // yellow
    {0.20, 0.80, 0.20}, // lime green
    {0.00, 0.39, 0.00}, // dark green
    {0.26, 0.70, 0.60}, // blue green
    {0.26, 0.96, 0.91}, // light blue
    {0.26, 0.65, 0.96}, // medium blue
    {0.04, 0.00, 0.80}, // navy blue
    {0.29, 0.00, 0.51}, // indigo
    {0.73, 0.33, 0.83}, // light purple
    {1.00, 0.00, 1.00}  // bright pink
};
const double NUM_BODIES = 40;
const double NUM_POINTS = 4;
double GRAVITY = 4000;
const double STAR_MASS = 20;

// Function prototypes.

/**
 * creates a star body of given points, size, and color 
 * moves star to given init_translation
 * @param init_translation the vector position to center the star at
 * @param color the color the star should be
 * @param points the number of visible points the star should have
 * @param outer_r the radius of the star from centroid to tip
 * @param inner_r the radius of the star from centroid to inner point
 * @return body_t star
 */
body_t *shape_init_star(vector_t init_translation,
                        rgb_color_t color,
                        size_t n_points,
                        double outer_r,
                        double inner_r);

/**
 * adds a single star to the scene provided
 * @param scene the scene to add the star to
 */
void n_scene_add_shape(scene_t *scene);

/**
 * calls the force_creator for Newtonian gravity between every
 *  pair of objects created in the scene.
 * @param scene the scene in which all the bodies created so far
 *              will have a gravitational force applied to them
 */
void n_scene_gravity(scene_t *scene);

// Function definitions.

body_t *shape_init_star(vector_t init_translation,
                        rgb_color_t color,
                        size_t n_points,
                        double outer_r,
                        double inner_r) {
    assert(n_points > 1);
    assert(outer_r > 0 && inner_r > 0);
    n_points = n_points * 2;

    // Star has double the points of outer points
    double angle = (2 * M_PI) / n_points;

    list_t *star = list_init(n_points, free);

    vector_t *p = NULL;
    for (size_t i = 0; i < n_points; i++) {
        p = malloc(sizeof(vector_t));
        assert(p != NULL);
        p->x = 0;
        p->y = ((i % 2 == 0) ? outer_r : inner_r);
        double alpha = i * angle;
        *p = vec_rotate(*p, alpha);
        list_add(star, p);
    }
    body_t *body = body_init(star, STAR_MASS, color);
    body_translate(body, init_translation);
    return body;
}

void n_scene_add_shape(scene_t *scene) {
    // initializes the stars to random locations within the bounds of the scene
    vector_t translate_by = {.x = (rand() / (double)RAND_MAX) * MAX_X,
                             .y = (rand() / (double)RAND_MAX) * MAX_Y};
    body_t *curr_star
        = shape_init_star(translate_by,
                          COLORS[scene_bodies(scene) % TOTAL_COLORS],
                          NUM_POINTS,
                          OUTER_R,
                          INNER_R);
    scene_add_body(scene, curr_star);
}

void n_scene_gravity(scene_t *scene) {
    for (size_t i = 0; i < (scene_bodies(scene)); i++) {
        for (size_t m = 0; m < (scene_bodies(scene)); m++)
            if (m != i) {
                create_newtonian_gravity(scene,
                                         GRAVITY,
                                         scene_get_body(scene, i),
                                         scene_get_body(scene, m));
            }
    }
}

int main(int argc, char *argv[]) {
    scene_t *s = scene_init();

    sdl_init(VEC_ZERO, SCENE_DIMS);
    double dt = 0.0;

    srand(time(NULL));

    for (int i = 0; i < NUM_BODIES; i++) {
        n_scene_add_shape(s);
    }

    n_scene_gravity(s);

    while (!sdl_is_done()) {
        dt = time_since_last_tick();
        scene_tick(s, dt);
        sdl_render_scene(s);
    }

    scene_free(s);

    return 0;
}
