/* Demo
- different collision
    - eating other objects
    - wrapping off screen
- needs to provide to scene:
    - collision
    - spawn
    - remove
    - color logic
*/

#include "body.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_X 0
#define MIN_Y 0
#define MAX_X 2000
#define MAX_Y 1000
const vector_t MIN = {MIN_X, MIN_Y};
const vector_t MAX = {MAX_X, MAX_Y};
const vector_t INITIAL_POSITION_PACMAN = {MAX_X / 2.0, MAX_Y / 2.0};
const double DEFAULT_SPEED = 40.0;
const double ACCELERATION_FACTOR = 100.0;
const double PACMAN_RADIUS = 50;
const double PACMAN_MOUTH_ANGLE = M_PI / 3;
const double PACMAN_MASS = 1;
const double PACMAN_RESOLUTION = 50;
const double PELLET_POINTS = 4;
const double PELLET_MASS = 1;
const double PELLET_RADIUS = 10;
const double PELLET_ADD_DELAY = 0.7;
const double PACMAN_BUFFER_RADIUS = 0.2;
const int PELLETS_AT_START = 10;
const rgb_color_t PACMAN_COLOR = {1.0, 0.64, 0.0};
#define TOTAL_COLORS 9
const rgb_color_t PELLET_COLORS[TOTAL_COLORS] = {
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

// Prototypes
void pacman_add_pellet(scene_t *world);
void pacman_add_pacman(scene_t *world);
body_t *pacman_init_pacman(double radius, vector_t initial_position);
body_t *pacman_init_pellet(double radius, int max_x, int max_y);
bool pacman_collision(body_t *pacman, body_t *curr_pellet);
void pacman_wrap(body_t *pacman, int min_x, int min_y, int max_x, int max_y);

// useless in this world
rgb_color_t pacman_color_logic(scene_t *world_state, body_t *shape);

// Functions

/**
 * pacman_add_pellet creates a pellet and passes the
 * shape back up to the world. Modifies world
 * @param world - a scene_t * that is editted
 */
void pacman_add_pellet(scene_t *world) {
    scene_add_body(world, pacman_init_pellet(PELLET_RADIUS, MAX.x, MAX.y));
}

/**
 * pacman_add_pacman creates a pacman and passes the
 * shape back up to the world. Modifies world
 * @param world - a scene_t * that is editted
 */
void pacman_add_pacman(scene_t *world) {
    if ((scene_bodies(world)) < 1) {
        scene_add_body(
            world,
            pacman_init_pacman(PACMAN_RADIUS, INITIAL_POSITION_PACMAN));
    }
}

/**
 * pacman_init_pacmam returns a new body_t of a pacman shape
 * initially, pac-man has 0 velocity, 0 angular-vel, 0 acceleration
 * @param radius the size of the pacman
 * @param initial_position where on the screen pacman starts the game
 * @return new pacman body_t *
 */
body_t *pacman_init_pacman(double radius, vector_t initial_position) {
    assert(radius > 0);

    list_t *pacman_points = list_init((PACMAN_RESOLUTION + 1), free);

    double angle_left = (2 * M_PI - PACMAN_MOUTH_ANGLE) / PACMAN_RESOLUTION;

    // initializes all of the vectors in the circular part of pacman,
    // centered (0,0)
    vector_t *p = NULL;
    for (size_t i = 0; i < PACMAN_RESOLUTION; i++) {
        p = malloc(sizeof(vector_t));
        assert(p != NULL);
        p->x = 0;
        p->y = radius;
        double curr_angle = i * angle_left;
        *p = vec_rotate(*p, curr_angle);
        list_add(pacman_points, p);
    }

    // add the center of the mouth:
    vector_t *center = malloc(sizeof(vector_t));
    center->x = 0;
    center->y = 0;
    list_add(pacman_points, center);

    // tilt pacman so its facing the right way
    polygon_rotate(pacman_points,
                   ((3 * M_PI / 2) + (PACMAN_MOUTH_ANGLE / 2)),
                   *center);

    body_t *pacman = body_init(pacman_points, PACMAN_MASS, PACMAN_COLOR);

    // move pacman to its initial location
    body_set_centroid(pacman, initial_position);

    return pacman;
}

/**
 * pacman_init_pellet returns a new body_t of a pellet shape
 * this pellet has 0 velocity, 0 angular-velocity, 0 acceleration
 * @param radius the double size that the pellet should be
 * @param max_x the dimensions of the world in the x direction
 * @param max_y the dimensions of the world in the y direction
 * @return new pellet body_t*
 */
body_t *pacman_init_pellet(double radius, int max_x, int max_y) {
    assert(radius > 0);
    assert(max_x > 0);
    assert(max_y > 0);

    list_t *pellet_points = list_init(PELLET_POINTS, free);

    double angle = 2 * M_PI / PELLET_POINTS;

    // initializes all of the vectors in the pellet, pellet centered (0,0)
    vector_t *p = NULL;
    for (size_t i = 0; i < PELLET_POINTS; i++) {
        p = malloc(sizeof(vector_t));
        assert(p != NULL);
        p->x = 0;
        p->y = radius;
        double curr_angle = i * angle;
        *p = vec_rotate(*p, curr_angle);
        list_add(pellet_points, p);
    }

    int random_int = rand();

    // if we include velocity and other things, set to 0
    body_t *pellet = body_init(pellet_points,
                               PELLET_MASS,
                               PELLET_COLORS[random_int % TOTAL_COLORS]);

    // initialize the pellet to a random location within the screen
    vector_t translate_by = {.x = (rand() / (double)RAND_MAX) * max_x,
                             .y = (rand() / (double)RAND_MAX) * max_y};
    body_translate(pellet, translate_by);

    return pellet;
}

/**
 * pacman_collision takes in a world state and a given shape and checks
 * if the given shape is colliding with anything in the world.
 * this function should be called in a for loop in world
 * @param pacman the pacman body_t
 * @param curr_pellet the current pellet in the world
 */
bool pacman_collision(body_t *pacman, body_t *curr_pellet) {
    // "eaten" if centroid of pellet overlaps with circle of pacman - buffer
    double buffer = PACMAN_BUFFER_RADIUS * PACMAN_RADIUS;
    vector_t pellet_center = body_get_centroid(curr_pellet);
    vector_t pacman_center = body_get_centroid(pacman);
    bool check_x
        = (fabs(pellet_center.x - pacman_center.x) < (PACMAN_RADIUS - buffer));
    bool check_y
        = (fabs(pellet_center.y - pacman_center.y) < (PACMAN_RADIUS - buffer));
    return (check_x && check_y);
}

/**
 * pacman_wrap checks if all of the points in the pacman object are
 * beyond the edge of the world. translates pacman to the otherside
 * of the screen if true, else does nothing
 * @param pacman the pacman object
 * @param min_x the min of the screen in the x direction
 * @param min_y the min of the screen in the y direction
 * @param max_x the max of the screen in the x direction
 * @param max_y the max of the screen in the y direction
 */
void pacman_wrap(body_t *pacman, int min_x, int min_y, int max_x, int max_y) {
    bool all_off_screen = true;
    for (int i = 0; i < PACMAN_RESOLUTION + 1; i++) {
        list_t *shape = body_get_shape(pacman);
        vector_t *curr_point = list_get(shape, i);
        if (!((curr_point->x < min_x) || (curr_point->y < min_y)
              || (curr_point->x > max_x) || (curr_point->y > max_y))) {
            all_off_screen = false;
            list_free(shape);
            break;
        } else {
            list_free(shape);
        }
    }
    if (all_off_screen) {
        vector_t centroid = body_get_centroid(pacman);
        vector_t translate_by;
        double width = max_x - min_x;
        double height = max_y - min_y;
        if (centroid.x > max_x) {
            translate_by = (vector_t){.x = -width, .y = 0};
        } else if (centroid.x < min_x) {
            translate_by = (vector_t){.x = width, .y = 0};
        } else if (centroid.y > max_y) {
            translate_by = (vector_t){.x = 0, .y = -height};
        } else if (centroid.y < min_y) {
            translate_by = (vector_t){.x = 0, .y = height};
        } else {
            assert(false && "pacman_wrap created an impossible case");
        }
        body_translate(pacman, translate_by);
    }
}

void pacman_on_key(char key,
                   key_event_type_t type,
                   double held_time,
                   body_t *pacman) {
    vector_t acceleration = VEC_ZERO;
    vector_t velocity = body_get_velocity(pacman);
    if (type == KEY_PRESSED) {
        double angle = body_get_rotation(pacman);
        switch (key) {
        case UP_ARROW:
            velocity.x = 0.0;
            angle = M_PI / 2;
            acceleration = (vector_t){.x = 0.0, .y = ACCELERATION_FACTOR};
            break;
        case DOWN_ARROW:
            velocity.x = 0.0;
            angle = (-1.0) * M_PI / 2;
            acceleration
                = (vector_t){.x = 0.0, .y = (-1.0) * ACCELERATION_FACTOR};
            break;
        case LEFT_ARROW:
            velocity.y = 0.0;
            angle = M_PI;
            acceleration
                = (vector_t){.x = (-1.0) * ACCELERATION_FACTOR, .y = 0.0};
            break;
        case RIGHT_ARROW:
            velocity.y = 0.0;
            angle = 0.0;
            acceleration = (vector_t){.x = ACCELERATION_FACTOR, .y = 0.0};
            break;
        }
        body_set_rotation(pacman, angle);
    }
    // set acceleration to 0
    else if (type == KEY_RELEASED) {
        switch (key) {
        case UP_ARROW:
            velocity = (vector_t){.x = 0.0, .y = DEFAULT_SPEED};
            break;
        case DOWN_ARROW:
            velocity = (vector_t){.x = 0.0, .y = (-1.0) * DEFAULT_SPEED};
            break;
        case LEFT_ARROW:
            velocity = (vector_t){.x = (-1.0) * DEFAULT_SPEED, .y = 0.0};
            break;
        case RIGHT_ARROW:
            velocity = (vector_t){.x = DEFAULT_SPEED, .y = 0.0};
            break;
        }
    }
    body_set_velocity(pacman, velocity);

    body_set_acceleration(pacman, vec_multiply(held_time, acceleration));
}

int main(int argc, char *argv[]) {
    scene_t *pacman_world = scene_init();
    scene_set_dims(pacman_world, MIN, MAX);
    srand(time(NULL));

    double dt = 0.0;

    sdl_init(scene_get_min(pacman_world), scene_get_max(pacman_world));

    pacman_add_pacman(pacman_world);
    body_t *pacman = scene_get_body(pacman_world, 0);
    sdl_on_key((key_handler_t)pacman_on_key, pacman);

    for (int i = 0; i < PELLETS_AT_START; i++) {
        pacman_add_pellet(pacman_world);
    }

    double time_since_last_add = 0.0;

    while (!sdl_is_done()) {

        dt = time_since_last_tick();

        if (time_since_last_add >= PELLET_ADD_DELAY) {
            pacman_add_pellet(pacman_world);
            time_since_last_add = 0.0;
        } else {
            time_since_last_add += dt;
        }

        for (int m = scene_bodies(pacman_world) - 1; m > 0; m--) {
            body_t *curr_pellet = scene_get_body(pacman_world, m);
            if (pacman_collision(pacman, curr_pellet)) {
                scene_remove_body(pacman_world, m);
            }
        }

        pacman_wrap(pacman, MIN_X, MIN_Y, MAX_X, MAX_Y);

        body_add_force(
            pacman,
            vec_multiply(PACMAN_MASS, body_get_acceleration(pacman)));

        scene_tick(
            pacman_world,
            dt); // does effectively nothing right now except update total time
        sdl_render_scene(pacman_world);
    }

    scene_free(pacman_world);

    return 0;
}
