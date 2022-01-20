#include "body.h"
#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#define STAR_MASS 100
const vector_t GRAVITY_FORCE = {.x = 0.0, .y = -50.0 * STAR_MASS};
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
const rgb_color_t COLORS[TOTAL_COLORS] = {
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

bool collision(vector_t min, vector_t max, body_t *shape);

void world_step(world_t *world, double dt);

void world_add_shape(world_t *world);

void world_remove_shape(world_t *world, size_t shape_idx);

bool world_should_add(world_t *world);

void shape_draw(body_t *shape);

void world_free(world_t *world);

void world_draw(world_t *world);

// Function definitions.

world_t *
world_init(vector_t gravity, vector_t min, vector_t max, size_t num_shapes) {
    world_t *w = malloc(sizeof(world_t));
    assert(w != NULL);
    w->shapes = list_init(num_shapes, body_free);
    w->max_n_shapes = num_shapes;
    w->min = min;
    w->max = max;
    w->gravity = gravity;
    w->time_since_last_add = 0;
    w->total_shapes_created = 0;

    return w;
}

bool collision(vector_t min, vector_t max, body_t *shape) {
    vector_t velocity = body_get_velocity(shape);
    list_t *poly = body_get_shape(shape);
    size_t i;
    size_t n = list_size(poly);
    vector_t *p;

    for (i = 0; i < n; i++) {
        p = list_get(poly, i);
        if (p->y <= min.y) {
            double diff = fabs(min.y - p->y);
            body_translate(shape, (vector_t){0, diff});
            if (velocity.y < 0) {
                velocity.y *= -ELASTICITY;
                if (fabs(velocity.y) < VELOCITY_THRESHOLD) {
                    velocity.y = 0;
                    body_add_force(shape, VEC_ZERO);
                    /// body_set_acceleration(shape, VEC_ZERO); // NEEDS
                    /// REDESIGN
                    body_set_angular_velocity(shape, 0);
                }
                body_set_velocity(shape, velocity);
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
        body_t *curr_shape = list_get(shapes, i);
        body_add_force(curr_shape, GRAVITY_FORCE);
        body_tick(curr_shape, dt);
        collision(world->min, world->max, curr_shape);
        // Check if shape has left the world.
        if (body_get_centroid(curr_shape).x - OUTER_R > world->max.x) {
            world_remove_shape(world, i);
        }
    }
}

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

void world_add_shape(world_t *world) {
    body_t *star
        = shape_init_star(INITIAL_POSITION,
                          COLORS[world->total_shapes_created % TOTAL_COLORS],
                          world->total_shapes_created + 2,
                          OUTER_R,
                          INNER_R);
    body_set_velocity(star, INITIAL_VELOCITY);
    body_set_angular_velocity(star, INITIAL_ANGULAR_VEL);
    body_add_force(star, world->gravity);

    list_add(world->shapes, star);
    world->total_shapes_created++;
    world->time_since_last_add = 0;
}

void world_remove_shape(world_t *world, size_t shape_idx) {
    body_free(list_remove(world->shapes, shape_idx));
}

bool world_should_add(world_t *world) {
    return list_size(world->shapes) == 0
           || (world->time_since_last_add >= SHAPE_ADD_DELAY
               && list_size(world->shapes) < world->max_n_shapes);
}

void shape_draw(body_t *shape) {
    sdl_draw_polygon(body_get_shape(shape), body_get_color(shape));
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
    world_t *w = world_init(GRAVITY_FORCE, VEC_ZERO, WORLD_DIMS, NUM_SHAPES);

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
