/**
 * Breakout Demo!
 *     move paddle back and forth to bounce ball
 *     if ball hits a brick, the brick disappears!
 *     you win if you can remove all the bricks
 *     the game resets if you miss the ball with your paddle
 *          (i.e. it hits the bottom of the screen)
 *
 * Special features:
 *  - paddle wraps around bottom of screen
 *  - lives of bricks with darker color == more lives
 *  - spin is added to ball such that the angle the ball bounces off the
 *      paddle changes with how you hit the ball. This is calculated
 *      from friction between the ball and paddle and angular impulse
 */

#include "body.h"
#include "collision.h"
#include "color.h"
#include "forces.h"
#include "list.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_X 0.0
#define MIN_Y 0.0
#define MAX_X 2000
#define MAX_Y 1000
const vector_t MIN = {MIN_X, MIN_Y};
const vector_t MAX = {MAX_X, MAX_Y};

const vector_t e1 = {.x = 1, .y = 0};
const vector_t e2 = {.x = 0, .y = 1};
const vector_t e3 = {.x = 1, .y = 1};

#define BALL_RADIUS 10
#define HEIGHT_BRICK 50
const double WIDTH_BRICK = 190;
const double BUFFER = 10;
const double BUFFER_CENTER = 1.0;
const double BALL_RESOLUTION = 20;
const vector_t INITIAL_POSITION_PADDLE
    = {.x = (MAX_X - MIN_X) / 2, .y = HEIGHT_BRICK / 2};
const vector_t INTIIAL_POSITION_BALL
    = {.x = (MAX_X - MIN_X) / 2, .y = HEIGHT_BRICK + BALL_RADIUS + 5};

const double PADDLE_MASS = INFINITY;
#define BALL_MASS 3
const double BRICK_MASS = INFINITY;
const double WALL_MASS = INFINITY;

#define SPEED_AMPLIFICATION_FACTOR 100
const double BALL_SPEED = 3 * SPEED_AMPLIFICATION_FACTOR;
const double PADDLE_SPEED = 15 * SPEED_AMPLIFICATION_FACTOR;

const double NUM_ROWS = 3;
const double WALL_THICKNESS = 400;

const double LIVES_ROW = 1;
const double LIVES_COL = 0;

const double ELASTICITY = 1.0;
const double FRICTION = 0.5;

const double BALL_INERTIA = 1.0 / 2 * BALL_MASS * BALL_RADIUS * BALL_RADIUS;

const rgb_color_t PADDLE_COLOR = {1.0, 0.0, 0.0};
const rgb_color_t BALL_COLOR = {1.0, 0.0, 0.0};
const rgb_color_t WALL_COLOR = {1.0, 1.0, 1.0};
const size_t TOTAL_COLORS = 10;
const rgb_color_t COLORS[] = {
    {1.00, 0.00, 0.00}, // red
    {0.96, 0.50, 0.26}, // orange
    {0.96, 0.96, 0.26}, // yellow
    {0.20, 0.80, 0.20}, // lime green
    {0.26, 0.70, 0.60}, // blue green
    {0.26, 0.96, 0.91}, // light blue
    {0.26, 0.65, 0.96}, // medium blue
    {0.04, 0.00, 0.80}, // navy blue
    {0.73, 0.33, 0.83}, // light purple
    {1.00, 0.00, 1.00}  // bright pink
};
const rgb_color_t SECONDARY_COLORS[] = {
    {0.90, 0.00, 0.00}, // red
    {0.86, 0.40, 0.16}, // orange
    {0.86, 0.86, 0.16}, // yellow
    {0.10, 0.70, 0.10}, // lime green
    {0.16, 0.60, 0.50}, // blue green
    {0.16, 0.86, 0.81}, // light blue
    {0.16, 0.55, 0.86}, // medium blue
    {0.00, 0.00, 0.70}, // navy blue
    {0.63, 0.23, 0.73}, // light purple
    {0.90, 0.00, 0.90}  // bright pink
};
const rgb_color_t TERTIARY_COLORS[] = {
    {0.80, 0.00, 0.00}, // red
    {0.76, 0.30, 0.06}, // orange
    {0.76, 0.76, 0.06}, // yellow
    {0.00, 0.60, 0.00}, // lime green
    {0.06, 0.50, 0.40}, // blue green
    {0.06, 0.76, 0.71}, // light blue
    {0.06, 0.45, 0.76}, // medium blue
    {0.00, 0.00, 0.60}, // navy blue
    {0.53, 0.13, 0.63}, // light purple
    {0.80, 0.00, 0.80}  // bright pink
};

// make the info type for body_init_with_info
typedef enum info { PADDLE_INFO, BALL_INFO, WALL_INFO, BRICK_INFO } info_e;
//                      0           1           2            3
//                      variable-lives bricks from 3 to 3 + n

// Typedefs
typedef struct breakout_key_aux {
    body_t *paddle;
} breakout_key_aux_t;

// Function Prototypes

/**
 * returns a rectangle of given height, width, color, mass, and position
 * centered at (0,0)
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
 * breakout_init_ball returns a new ball at a given intial position
 * @param initial_position the location of the centroid of the new ball
 */
body_t *breakout_init_ball(vector_t initial_position);

/**
 * breakout_init_paddle returns a new paddle at a given intial position
 * @param initial_position the location of the centroid of the new paddle
 */
body_t *breakout_init_paddle(vector_t initial_position);

/**
 * breakout_init_brick returns a new brick at a given intial position, with info
 * (e.g. brick lives determined by row, col)
 * @param initial_position the location of the centroid of the new brick
 * @param color the color of the new brick
 * @param row the row of the brick
 * @param col the col of the brick
 */
body_t *breakout_init_brick(vector_t initial_position,
                            rgb_color_t color,
                            int row,
                            int col);

/**
 * breakout_init_wall returns a new wall at a given intial position
 * @param initial_position the location of the centroid of the new wall
 * @param height the height of the wall
 * @param width the width of the wall
 */
body_t *
breakout_init_wall(vector_t initial_position, double height, double width);

/**
 * initializes the starting frame of the world with a paddle and ball in the
 * bottom center, a given number of rows of bricks at the top, and all the walls
 * @param b_world the scene_t world
 * @param brick_rows the number of rows of bricks that should be created
 */
void breakout_world_init(scene_t *b_world,
                         double brick_rows,
                         breakout_key_aux_t *key_aux);

/**
 * breakout_reset_game resets the game instead of ending it when the paddle
 * misses the ball / when the ball collides with the bottom wall
 * @param world the scene in which the game is being run
 */
void breakout_reset_game(scene_t *world, breakout_key_aux_t *key_aux);

/**
 * returns true if there are no bricks left on screen
 * returns false if there is at least one brick on screen
 * returns false if the game is reset
 * @param world the scene in which the game is being run
 */
bool breakout_end_game(scene_t *world, breakout_key_aux_t *key_aux);

/**
 * Initialize a key aux.
 * Free a breakout_key_aux_t with `free`.
 * @param scene the scene the aux is attached to
 * @return a new key aux.
 */
breakout_key_aux_t *breakout_key_aux_init(scene_t *scene);

/**
 * Get the paddle of the game.
 * @param scene the scene in which the paddle is defined
 * @return paddle body of scene, or NULL if the paddle hasn't been created yet
 */
body_t *breakout_get_paddle(scene_t *scene);

/**
 * Get the ball of the game.
 * @param scene the scene in which the ball is defined
 * @return ball body of scene, or NULL if the ball hasn't been created yet
 */
body_t *breakout_get_ball(scene_t *scene);

/**
 * handles key strokes for the scene, only moves the paddle body
 * paddle moves left and right with arrows
 * @param key the actual key pressed (e.g. space, right arrow)
 * @param type the type of key event (e.g. key-release)
 * @param held_time the amount of time the key has been held
 * @param aux the auxillary key struct with additional information
 */
void breakout_on_key(char key,
                     key_event_type_t type,
                     double held_time,
                     breakout_key_aux_t *aux);

/**
 * breakout_boundary_paddle wraps the paddle around the screen
 * if the paddle goes off the left side of the screen, it reappears
 * on the right side, and vice versa.
 * @param body the paddle that should be wrapped
 */
void breakout_boundary_paddle(body_t *body);

/**
 * Handle collision of ball with bricks that have multiple lives.
 * (Similar to collision_handler_asymmetric_destructive.)
 * @param body_to_remove the body (i.e. brick) to remove when lives are up
 * @param body_other the other body, i.e. ball
 * @param axis not needed here
 * @param aux not needed here
 */
void collision_handler_brick_lives(body_t *body_to_remove,
                                   body_t *body_other,
                                   vector_t axis,
                                   vector_t collision_point,
                                   aux_t *aux);

// Function Definitions

body_t *make_rectangle(rgb_color_t color,
                       vector_t initial_position,
                       double height,
                       double width,
                       double mass,
                       void *rec_info,
                       free_func_t freer) {
    list_t *rectangle_points = list_init(4, free);

    vector_t *left = malloc(sizeof(vector_t));
    assert(left != NULL);
    left->x = -width / 2.0;
    left->y = -height / 2.0;
    list_add(rectangle_points, left);

    vector_t *right = malloc(sizeof(vector_t));
    assert(right != NULL);
    right->x = width / 2.0;
    right->y = -height / 2.0;
    list_add(rectangle_points, right);

    vector_t *top_r = malloc(sizeof(vector_t));
    assert(top_r != NULL);
    top_r->x = width / 2.0;
    top_r->y = height / 2.0;
    list_add(rectangle_points, top_r);

    vector_t *top_l = malloc(sizeof(vector_t));
    assert(top_l != NULL);
    top_l->x = -width / 2.0;
    top_l->y = height / 2.0;
    list_add(rectangle_points, top_l);

    body_t *rectangle
        = body_init_with_info(rectangle_points, mass, color, rec_info, freer);

    body_set_centroid(rectangle, initial_position);

    return rectangle;
}

body_t *make_triangle(rgb_color_t color,
                      vector_t initial_position,
                      double size,
                      double mass,
                      void *tri_info,
                      free_func_t freer) {
    list_t *triangle_points = list_init(3, free);

    vector_t *left = malloc(sizeof(vector_t));
    assert(left != NULL);
    left->x = size / 2.0;
    left->y = 0.0;
    list_add(triangle_points, left);

    vector_t *right = malloc(sizeof(vector_t));
    assert(right != NULL);
    right->x = -size / 2.0;
    right->y = 0.0;
    list_add(triangle_points, right);

    vector_t *tip = malloc(sizeof(vector_t));
    assert(tip != NULL);
    tip->x = 0.0;
    tip->y = size / 2.0;
    list_add(triangle_points, tip);

    body_t *triangle
        = body_init_with_info(triangle_points, mass, color, tri_info, freer);

    body_set_centroid(triangle, initial_position);

    return triangle;
}

body_t *make_circle(double radius,
                    double resolution,
                    vector_t initial_position,
                    double mass,
                    void *circle_info,
                    free_func_t freer,
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

    body_t *ball
        = body_init_with_info(ball_points, mass, color, circle_info, freer);
    body_set_centroid(ball, initial_position);

    return ball;
}

body_t *breakout_init_ball(vector_t initial_position) {
    enum info *curr_info = malloc(sizeof(int));
    *curr_info = BALL_INFO;

    body_t *b = make_circle(BALL_RADIUS,
                            BALL_RESOLUTION,
                            initial_position,
                            BALL_MASS,
                            curr_info,
                            free,
                            BALL_COLOR);

    body_set_inertia(b, BALL_INERTIA);
    return b;
}

body_t *breakout_init_paddle(vector_t initial_position) {
    enum info *curr_info = malloc(sizeof(int));
    *curr_info = PADDLE_INFO;
    return make_rectangle(PADDLE_COLOR,
                          INITIAL_POSITION_PADDLE,
                          HEIGHT_BRICK,
                          WIDTH_BRICK,
                          PADDLE_MASS,
                          curr_info,
                          free);
}

body_t *breakout_init_brick(vector_t initial_position,
                            rgb_color_t color,
                            int row,
                            int col) {
    enum info *curr_info = malloc(sizeof(int));
    *curr_info = (int)BRICK_INFO + (LIVES_ROW * row) + (LIVES_COL * col);
    return make_rectangle(color,
                          initial_position,
                          HEIGHT_BRICK,
                          WIDTH_BRICK,
                          BRICK_MASS,
                          curr_info,
                          free);
}

body_t *
breakout_init_wall(vector_t initial_position, double height, double width) {
    enum info *curr_info = malloc(sizeof(int));
    *curr_info = WALL_INFO;
    return make_rectangle(WALL_COLOR,
                          initial_position,
                          height,
                          width,
                          WALL_MASS,
                          curr_info,
                          free);
}

void breakout_world_init(scene_t *b_world,
                         double brick_rows,
                         breakout_key_aux_t *key_aux) {
    // add the paddle and ball
    body_t *paddle = breakout_init_paddle(INITIAL_POSITION_PADDLE);
    scene_add_body(b_world, paddle); // Position 0 in scene bodies.
    body_t *ball = breakout_init_ball(INTIIAL_POSITION_BALL);
    scene_add_body(b_world, ball); // Position 1 in scene bodies.
    body_set_velocity(ball, vec_multiply(BALL_SPEED, e3));

    // add non-destructive collisions between ball and paddle
    create_physics_spin_collision(b_world, ELASTICITY, FRICTION, ball, paddle);

    // add the walls to the scene
    body_t *left_wall
        = breakout_init_wall((vector_t){-WALL_THICKNESS / 2, MAX_Y / 2},
                             MAX_Y,
                             WALL_THICKNESS);
    body_t *right_wall
        = breakout_init_wall((vector_t){WALL_THICKNESS / 2 + MAX_X, MAX_Y / 2},
                             MAX_Y,
                             WALL_THICKNESS);
    body_t *top_wall
        = breakout_init_wall((vector_t){MAX_X / 2, WALL_THICKNESS / 2 + MAX_Y},
                             WALL_THICKNESS,
                             MAX_X);
    body_t *bottom_wall
        = breakout_init_wall((vector_t){MAX_X / 2, -WALL_THICKNESS / 2},
                             WALL_THICKNESS,
                             MAX_X);
    scene_add_body(b_world, left_wall);   // Position 2 in scene bodies.
    scene_add_body(b_world, right_wall);  // Position 3.
    scene_add_body(b_world, top_wall);    // Position 4.
    scene_add_body(b_world, bottom_wall); // Position 5.

    // add collisions between paddle and walls, and ball and walls
    create_physics_spin_collision(b_world,
                                  ELASTICITY,
                                  FRICTION,
                                  ball,
                                  left_wall);
    create_physics_spin_collision(b_world,
                                  ELASTICITY,
                                  FRICTION,
                                  ball,
                                  right_wall);
    create_physics_spin_collision(b_world,
                                  ELASTICITY,
                                  FRICTION,
                                  ball,
                                  top_wall);
    create_asymmetric_destructive_collision(b_world, ball, bottom_wall);

    list_t *boundary_paddle_list = list_init(1, NULL);
    list_add(boundary_paddle_list, paddle);
    scene_add_bodies_force_creator(b_world,
                                   (force_creator_t)breakout_boundary_paddle,
                                   paddle,
                                   boundary_paddle_list,
                                   NULL);

    // calculate the num of bricks that can fit in a row based on effective size
    double width_brick = WIDTH_BRICK + BUFFER;
    int num_in_a_row = (int)(MAX_X - MIN_X) / width_brick;

    // add all the bricks to the screen
    for (int row = 0; row < brick_rows; row++) {
        for (int position_in_row = 0; position_in_row < num_in_a_row;
             position_in_row++) {
            vector_t initial_position
                = {.x = BUFFER_CENTER + WIDTH_BRICK / 2.0
                        + (position_in_row * width_brick),
                   .y = MAX_Y - (HEIGHT_BRICK / 2.0) - (row * HEIGHT_BRICK)
                        - (BUFFER * row)};
            rgb_color_t curr_color;
            if (row == (brick_rows - 1)) {
                curr_color = COLORS[position_in_row % num_in_a_row];
            } else if (row == (brick_rows - 2)) {
                curr_color = SECONDARY_COLORS[position_in_row % num_in_a_row];
            } else {
                curr_color = TERTIARY_COLORS[position_in_row % num_in_a_row];
            }
            body_t *curr_brick = breakout_init_brick(initial_position,
                                                     curr_color,
                                                     (brick_rows - row - 1),
                                                     position_in_row);
            scene_add_body(b_world, curr_brick);
            create_physics_spin_collision(b_world,
                                          ELASTICITY,
                                          FRICTION,
                                          curr_brick,
                                          ball);
            create_collision(b_world,
                             curr_brick,
                             ball,
                             (collision_handler_t)collision_handler_brick_lives,
                             NULL,
                             NULL);
        }
    }
    // other parts of the world that need to be remade upon reset only but not
    // upon init
    if (key_aux != NULL) {
        key_aux->paddle = paddle;
    }
}

void collision_handler_brick_lives(body_t *body_to_remove,
                                   body_t *body_other,
                                   vector_t axis,
                                   vector_t collision_point,
                                   aux_t *aux) {
    int *lives_p = (int *)body_get_info(body_to_remove);
    int lives = (*lives_p) - 3;
    if (lives == 0) {
        body_remove(body_to_remove);
        // DO NOT REMOVE OTHER BODY
    } else {
        (*lives_p)--;
    }
}

void breakout_reset_game(scene_t *world, breakout_key_aux_t *key_aux) {
    // remove all the current bodies in the world
    for (size_t i = 0; i < scene_bodies(world); i++) {
        body_remove(scene_get_body(world, i));
    }
    breakout_world_init(world, NUM_ROWS, key_aux);
}

bool breakout_end_game(scene_t *world, breakout_key_aux_t *key_aux) {
    // reset the game if the ball is marked for removal (i.e. you miss the ball)
    if ((breakout_get_ball(world) == NULL)
        || (body_is_removed(breakout_get_ball(world)))) {
        breakout_reset_game(world, key_aux);
        return false;
    } else {
        for (size_t i = 0; i < scene_bodies(world); i++) {
            body_t *curr_body = scene_get_body(world, i);
            if (*(info_e *)body_get_info(curr_body) == BRICK_INFO) {
                return false;
            }
        }
    }
    // there are no more bricks to hit, the player has won
    printf("Congrats you won!");
    return true;
}

breakout_key_aux_t *breakout_key_aux_init(scene_t *scene) {
    breakout_key_aux_t *aux = malloc(sizeof(breakout_key_aux_t));
    aux->paddle = breakout_get_paddle(scene);
    return aux;
}

body_t *breakout_get_paddle(scene_t *scene) {
    body_t *paddle = scene_get_body(scene, 0);
    if (*(info_e *)body_get_info(paddle) == PADDLE_INFO) {
        return paddle;
    }
    return NULL;
}

body_t *breakout_get_ball(scene_t *scene) {
    body_t *ball = scene_get_body(scene, 1);
    if (*(info_e *)body_get_info(ball) == BALL_INFO) {
        return ball;
    }
    return NULL;
}

void breakout_on_key(char key,
                     key_event_type_t type,
                     double held_time,
                     breakout_key_aux_t *aux) {
    body_t *paddle = aux->paddle;
    switch (key) {
    case LEFT_ARROW:
        switch (type) {
        case KEY_PRESSED: // Move left.
            body_set_velocity(paddle, vec_multiply(-PADDLE_SPEED, e1));
            break;
        case KEY_RELEASED: // Stop moving.
            body_set_velocity(paddle, VEC_ZERO);
            break;
        }
        break;
    case RIGHT_ARROW:
        switch (type) {
        case KEY_PRESSED: // Move right.
            body_set_velocity(paddle, vec_multiply(PADDLE_SPEED, e1));
            break;
        case KEY_RELEASED: // Stop moving.
            body_set_velocity(paddle, VEC_ZERO);
            break;
        }
        break;
    }
}

void breakout_boundary_paddle(body_t *body) {
    const double rebound_factor = MAX_X - MIN_X - WIDTH_BRICK;
    list_t *body_shape = body_get_shape(body);
    for (size_t i = 0; i < list_size(body_shape); i++) {
        vector_t *curr_vertex = list_get(body_shape, i);
        if (curr_vertex->x > MAX_X) {
            body_translate(body, vec_multiply((-rebound_factor), e1));
            break;
        } else if (curr_vertex->x < MIN_X) {
            body_translate(body, vec_multiply(rebound_factor, e1));
            break;
        }
    }
    list_free(body_shape);
}

int main(int argc, char **argv) {

    scene_t *b_world = scene_init();
    scene_set_dims(b_world, MIN, MAX);
    // initially, the world_init shouldn't be able to adjust the key_aux
    breakout_world_init(b_world, NUM_ROWS, NULL);

    breakout_key_aux_t *key_aux = breakout_key_aux_init(b_world);
    sdl_on_key((key_handler_t)breakout_on_key, key_aux);

    double dt = 0.0;

    sdl_init(MIN, MAX);

    while (!sdl_is_done() && !breakout_end_game(b_world, key_aux)) {
        dt = time_since_last_tick();
        scene_tick(b_world, dt);
        sdl_render_scene(b_world);
    }

    free(key_aux);
    scene_free(b_world);

    return 0;
}
