#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "shape.h"
#include "vec_list.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

const vector_t GRAVITY = {.x = 0.0, .y = -50.0};
#define MAX_X 2000
#define MAX_Y 1000
const vector_t WORLD_DIMS = {.x = MAX_X, .y = MAX_Y};
#define NUM_SHAPES 10
#define OUTER_R 100
#define INNER_R 50
const double VELOCITY_THRESHOLD = 1;
const double INITIAL_ANGULAR_VEL = 0.95;
const double ELASTICITY = 0.7;
const vector_t INITIAL_POSITION = {.x = OUTER_R + 1, .y = MAX_Y - OUTER_R - 1};
#define INITIAL_VELOCITY_X 100.0
#define INITIAL_VELOCITY_Y 0.0
const vector_t INITIAL_VELOCITY
    = {.x = INITIAL_VELOCITY_X, .y = INITIAL_VELOCITY_Y};
const double SHAPE_ADD_DELAY
    = MAX_X / NUM_SHAPES / INITIAL_VELOCITY_X; // Seconds
#define TOTAL_COLORS 12
const float COLORS[TOTAL_COLORS][3] = {
    {1.00, 0.00, 0.00}, // red
    {0.96, 0.60, 0.26}, // orange
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

typedef struct world {
    vector_t gravity;
    list_t *shapes;
    vector_t min;
    vector_t max;
    size_t max_n_shapes;
    // size_t curr_n_shapes; now in list
    double time_since_last_add;
    size_t total_shapes_created;
} world_t;

// Function prototypes.
world_t *
world_init(vector_t gravity, vector_t min, vector_t max, size_t num_shapes);

bool collision(vector_t min, vector_t max, shape_t *shape);

void world_step(world_t *world, double dt);

void world_add_shape(world_t *world);

void world_remove_shape(world_t *world, size_t shape_idx);

bool world_should_add(world_t *world);

void shape_draw(shape_t *shape);

void world_free(world_t *world);

void world_draw(world_t *world);

// Function definitions.

world_t *
world_init(vector_t gravity, vector_t min, vector_t max, size_t num_shapes) {
    world_t *w = malloc(sizeof(world_t));
    assert(w != NULL);
    w->shapes = list_init(num_shapes,
                          shape_free); // calloc(num_shapes, sizeof(shape_t *));
    // assert(w->shapes != NULL);
    w->max_n_shapes = num_shapes;
    w->min = min;
    w->max = max;
    w->gravity = gravity;
    w->time_since_last_add = 0;
    w->total_shapes_created = 0;

    return w;
}

bool collision(vector_t min, vector_t max, shape_t *shape) {
    vector_t velocity = shape_velocity(shape);
    vec_list_t *poly = shape_polygon(shape);
    size_t i;
    size_t n = vec_list_size(poly);
    vector_t *p;

    for (i = 0; i < n; i++) {
        p = vec_list_get(poly, i);
        if (p->y <= min.y) {
            double diff = fabs(min.y - p->y);
            shape_translate(shape, (vector_t){0, diff});
            if (velocity.y < 0) {
                velocity.y *= -shape_elasticity(shape);
                if (fabs(velocity.y) < VELOCITY_THRESHOLD) {
                    velocity.y = 0;
                    shape_set_acceleration(shape, VEC_ZERO);
                    shape_set_angular_vel(shape, 0);
                }
                shape_set_velocity(shape, velocity);
                return true;
            }
        }
    }

    return false;
}

void world_step(world_t *world, double dt) {
    list_t *shapes = world->shapes;
    if (world_should_add(world)) {
        world_add_shape(world);
    }
    for (size_t i = 0; i < list_size(shapes); i++) {
        shape_t *curr_shape = list_get(shapes, i);
        shape_step(curr_shape, dt);
        collision(world->min, world->max, curr_shape);
        // Check if shape has left the world.
        if (shape_centroid(curr_shape).x - OUTER_R > world->max.x) {
            world_remove_shape(world, i);
        }
    }
}

void world_add_shape(world_t *world) {
    list_add(world->shapes,
             shape_init_star(
                 INITIAL_VELOCITY,
                 INITIAL_ANGULAR_VEL,
                 world->gravity,
                 INITIAL_POSITION,
                 ELASTICITY,
                 (float *)COLORS[world->total_shapes_created % TOTAL_COLORS],
                 world->total_shapes_created + 2,
                 OUTER_R,
                 INNER_R));
    world->total_shapes_created++;
    world->time_since_last_add = 0;
}

void world_remove_shape(world_t *world, size_t shape_idx) {
    shape_free(list_remove(world->shapes, shape_idx));
}

bool world_should_add(world_t *world) {
    return list_size(world->shapes) == 0
           || (world->time_since_last_add >= SHAPE_ADD_DELAY
               && list_size(world->shapes) < world->max_n_shapes);
}

void shape_draw(shape_t *shape) {
    sdl_draw_polygon(shape_polygon(shape),
                     shape_rgb(shape)[0],
                     shape_rgb(shape)[1],
                     shape_rgb(shape)[2]);
}

void world_draw(world_t *world) {
    list_t *shapes = world->shapes;
    for (size_t i = 0; i < list_size(shapes); i++) {
        shape_draw(list_get(shapes, i));
    }
}

void world_free(world_t *world) {
    list_free(world->shapes);
    free(world);
}

int main(int argc, char *argv[]) {
    world_t *w = world_init(GRAVITY, VEC_ZERO, WORLD_DIMS, NUM_SHAPES);

    sdl_init(VEC_ZERO, WORLD_DIMS);
    double dt;

    while (!sdl_is_done()) {
        dt = time_since_last_tick();
        w->time_since_last_add += dt;
        world_step(w, dt);
        sdl_clear();
        world_draw(w);
        sdl_show();
    }

    world_free(w);

    return 0;
}
