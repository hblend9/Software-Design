/**
 * DEMO : spring forces and drag/damping
 *   no shapes ever get added or removed after initialization
 *   no collisions
 *   uses invisible bodies on a line at the equilibrium height to
 *      correctly model the y-direction only spring force
 *   right half of screen is damped oscillation, left side is not damped
 */

#include "body.h"
#include "forces.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN_X 0
#define MIN_Y 0
#define MAX_X 2000
#define MAX_Y 1000
const vector_t MIN = {MIN_X, MIN_Y};
const vector_t MAX = {MAX_X, MAX_Y};
const double RADIUS = 25;
const double RESOLUTION = 20;
const double BUFFER = 1;
const double MASS = 100;
const double INITIAL_HEIGHT = (MAX_Y - MIN_Y) / 2;
const double INVISIBLE_RADIUS = 5;
const double INVISIBLE_RESOLUTION = 4;
const double INVISIBLE_MASS = INFINITY;

const double ELASTICITY_K = 50;
const double GAMMA_DRAG = 5;

const rgb_color_t INVISIBLE_COLOR = {1.00, 1.00, 1.00};
#define TOTAL_COLORS 14
const rgb_color_t COLORS[TOTAL_COLORS] = {
    {1.00, 0.00, 0.00}, // red
    {0.98, 0.23, 0.00}, // red orange
    {0.96, 0.50, 0.26}, // orange
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

// Prototypes:

/**
 * calculates the number of balls that will fit in the x direction
 * on screen given the initial parameters defined at the top
 * @param void
 * @return int the number of balls that should be initialized
 */
int how_many_balls_on_screen();

/**
 * creates a ball body of given radius, mass, and color
 * and then moves it to its initial_position on screen
 * @param radius the radius of the ball
 * @param resolution how 'circular' the many sided polygon should look
 * @param initial_position the position the center of the ball will be
 * initialized to
 * @param mass the mass of the body to be created
 * @param color the color of the body to be created
 * @return body pointer to the ball that was initialized
 */
body_t *sd_init_ball(double radius,
                     double resolution,
                     vector_t initial_position,
                     double mass,
                     rgb_color_t color);

/**
 * initializes NUM_BODIES number of balls to a straight line at the equilibrium
 * height, these balls will not move. Useful for checking graphics and forces
 * @param world the scene in which the balls will be added
 */
void sd_add_balls_equilibrium(scene_t *world);

/**
 * initializes NUM_BODIES number of balls in a sinusoidal wave above equilibrium
 * height, as well as NUM_BODIES number of invisible balls at the equilibrium
 * height
 * @param world the scene in which the balls will be added
 * @param max_y the height the top of the sine wave of balls should touch
 * @param min_y the height the bottom of the sine wave of balls should touch
 */
void sd_add_balls_initial_state(scene_t *world, double max_y, double min_y);

// Functions:

int how_many_balls_on_screen() {
    int ball_diameter = 2 * RADIUS + BUFFER;
    // int division means round up manually
    return ((MAX_X - MIN_X) / ball_diameter + 1);
}

body_t *sd_init_ball(double radius,
                     double resolution,
                     vector_t initial_position,
                     double mass,
                     rgb_color_t color) {
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
        list_add(ball_points, p);
    }

    body_t *ball = body_init(ball_points, mass, color);
    body_translate(ball, initial_position);

    return ball;
}

void sd_add_balls_equilibrium(scene_t *world) {
    int num_balls = how_many_balls_on_screen();
    double width_interval = (MAX_X - MIN_X) / num_balls;
    for (int i = 0; i < num_balls; i++) {
        double x = (width_interval * i) + RADIUS;
        vector_t initial_position = {.x = x, .y = INITIAL_HEIGHT};
        rgb_color_t color = COLORS[i % TOTAL_COLORS];
        body_t *curr_ball
            = sd_init_ball(RADIUS, RESOLUTION, initial_position, MASS, color);
        scene_add_body(world, curr_ball);
    }
}

void sd_add_balls_initial_state(scene_t *world, double max_y, double min_y) {
    int num_balls = how_many_balls_on_screen();
    // reaches end to end x direction, not necessarily in the y direction
    double width_interval = (MAX_X - MIN_X) / num_balls;
    // sinusodial height
    double x_midpoint = (MAX_X - MIN_X) / 2.0;
    double y_midpoint = (max_y - min_y) / 2.0;
    double y_amplitude = max_y - y_midpoint;
    double w = M_PI / 2.0 / x_midpoint;
    for (int i = 0; i < num_balls; i++) {
        double x = (width_interval * i) + RADIUS;
        double y = y_amplitude * pow(sin(w * (x - x_midpoint)), 2) + y_midpoint;
        vector_t initial_position = {.x = x, .y = y};
        rgb_color_t color = COLORS[i % TOTAL_COLORS];
        body_t *curr_ball
            = sd_init_ball(RADIUS, RESOLUTION, initial_position, MASS, color);
        scene_add_body(world, curr_ball);
        vector_t initial_position_invisible = {.x = x, .y = y_midpoint};
        body_t *next_invisible = sd_init_ball(INVISIBLE_RADIUS,
                                              INVISIBLE_RESOLUTION,
                                              initial_position_invisible,
                                              INVISIBLE_MASS,
                                              INVISIBLE_COLOR);
        scene_add_body(world, next_invisible);
    }
}

int main(int argc, char *argv[]) {
    scene_t *sd_world = scene_init();

    double dt = 0.0;

    sdl_init(MIN, MAX);

    sd_add_balls_initial_state(sd_world, MAX_Y, MIN_Y);

    for (int m = 0; m < scene_bodies(sd_world); m++) {
        body_t *curr_ball = scene_get_body(sd_world, m);

        // right half of screen should damp, left side should not
        if (m > (scene_bodies(sd_world) / 2)) {
            create_drag(sd_world, GAMMA_DRAG, curr_ball);
        }
        // only apply spring forces to the "real" objects on screen,
        // don't add forces to the invisible ones
        if ((m != scene_bodies(sd_world) - 1) && (m % 2 == 0)) {
            body_t *next_invis = scene_get_body(sd_world, m + 1);
            create_spring(sd_world, ELASTICITY_K, curr_ball, next_invis);
        }
    }

    while (!sdl_is_done()) {
        dt = time_since_last_tick();
        scene_tick(sd_world, dt);
        sdl_render_scene(sd_world);
    }

    scene_free(sd_world);

    return 0;
}
