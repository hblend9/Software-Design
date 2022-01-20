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
#define MAX_X 1000
#define MAX_Y 1100
const vector_t MIN = {MIN_X, MIN_Y};
const vector_t MAX = {MAX_X, MAX_Y};

const vector_t e1 = {.x = 1, .y = 0};
const vector_t e2 = {.x = 0, .y = 1};

#define SPEED_AMPLIFICATION_FACTOR 100
const double ENEMY_SPEED = 1 * SPEED_AMPLIFICATION_FACTOR;
const double PLAYER_SPEED = 3 * SPEED_AMPLIFICATION_FACTOR;
const double LASER_SPEED = 5 * SPEED_AMPLIFICATION_FACTOR;

const double ENEMY_MASS = 1;
const double PLAYER_MASS = 1;
const double LASER_MASS = 1;

const rgb_color_t ENEMY_COLOR = {0.5, 0.5, 0.5};
const rgb_color_t PLAYER_COLOR = {0.0, 0.5, 0.0};
const rgb_color_t ENEMY_LASER_COLOR = {1.0, 0.0, 0.0};
const rgb_color_t PLAYER_LASER_COLOR = {0.0, 1.0, 0.0};
const rgb_color_t WALL_COLOR = {1.0, 1.0, 1.0};

const double ENEMY_SIZE = 100;
const double BUFFER = 10;
const double PLAYER_SIZE = 150;
const double LASER_SIZE = 10;

const double NUM_ROWS = 3;
const double WALL_THICKNESS = 400;

const double ENEMY_RESOLUTION = 20;

// make the info type for body_init_with_info
typedef enum info {
    PLAYER_INFO,
    ENEMY_INFO,
    PLAYER_LASER_INFO,
    ENEMY_LASER_INFO,
    WALL_INFO
} info_e;

const double PLAYER_LASER_DELAY = 0.25; // seconds
const double ENEMY_LASER_DELAY = 0.75;  // seconds

const double BUFFER_CENTER = 1.0;

// Typedefs
typedef struct si_key_aux {
    scene_t *scene;
    double time_since_last_shoot;
    body_t *player;
} si_key_aux_t;

// Function Prototypes

/**
 * si_enemy_init returns a new body_t enemy at given initial position
 * @param initial_position where on the screen enemy starts the game
 * @return new enemy body_t *
 */
body_t *si_enemy_init(vector_t initial_position);

/**
 * si_player_init returns a new body_t player at given initial position
 * @param initial_position where on the screen player starts the game
 * @return new player body_t *
 */
body_t *si_player_init(vector_t initial_position);

/**
 * si_init_laser returns a new body_t of a laser shape
 * @param color the color, either player or enemy, that the laser should be
 * @param initial_position the initial_position the laser should start at
 * @return new laser body_t*
 */
body_t *si_init_laser(rgb_color_t color,
                      vector_t initial_position,
                      void *laser_info,
                      free_func_t freer);

/**
 * si_init_laser_enemy returns a new body_t of a laser shape
 * @param initial_position the initial_position the laser should start at
 * @return new laser body_t*
 */
body_t *si_init_laser_enemy(vector_t initial_position);

/**
 * si_init_laser_player returns a new body_t of a laser shape
 * @param initial_position the initial_position the laser should start at
 * @return new laser body_t*
 */
body_t *si_init_laser_player(vector_t initial_position);

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
 * initializes the starting frame of the world with a player in the bottom left,
 * a given number of rows of enemy ships at the top, and all the walls
 * @param si_world the scene_t world
 * @param enemy_rows the number of rows of enemy ships that should be created
 */
void si_world_init(scene_t *si_world, double enemy_rows);

/**
 * returns true if the player no longer exists (hit by enemy laser or enemy
 * ships reach bottom of screen) or if the player has killed all the enemy ships
 * on screen returns false if there is a player and at least one enemy on screen
 * @param world the scene in which the game is being run
 */
bool end_game(scene_t *world);

/**
 * Adds a force creator to a scene that handles body collisions with the edges
 * of the scene.
 * @param scene the scene containing the bodies
 * @param body a body that the collisions deal with
 * @param n_consts the number of constants needed (can be 0)
 * @param ... variable number of constants, must match n_consts
 */
void si_create_wall_collision(scene_t *scene, body_t *body, int n_consts, ...);

/**
 * handles enemy collisions with the left, right and bottom walls.
 * if the enemy collides with the bottom wall, end the game
 * if the enemy collides with the left or right, move down a row
 * @param aux the auxillary struct with additional information
 */
void si_force_creator_enemy_wall_collision(aux_t *aux);

/**
 * handles player collisions with the left and right walls
 * prevents the player from moving past the left or right
 * @param aux the auxillary struct with additional information
 */
void si_force_creator_player_wall_collision(aux_t *aux);

/**
 * handles laser collisiions with the top and bottom walls
 * removes the laser once it nearly goes off screen
 * @param aux the auxillary struct with additional information
 */
void si_force_creator_laser_wall_collision(aux_t *aux);

/**
 * Initialize a key aux.
 * Free a si_key_aux_t with `free`.
 * @param scene the scene the aux is attached to
 * @return a new key aux.
 */
si_key_aux_t *si_key_aux_init(scene_t *scene);

/**
 * Get the player of the game.
 * @param scene the spaceinvaders player
 * @return player body of scene, or NULL if the player has been deleted
 */
body_t *si_get_player(scene_t *scene);

/**
 * Increment the state of a key aux forward one tick.
 * @param aux the key aux to tick
 * @param dt time since last tick
 */
void si_key_aux_tick(si_key_aux_t *aux, double dt);

/**
 * handles key strokes for the scene, only moves the player body
 * player shoots on space, left and right with arrows
 * @param key the actual key pressed (e.g. space, right arrow)
 * @param type the type of key event (e.g. key-release)
 * @param held_time the amount of time the key has been held
 * @param aux the auxillary key struct with additional information
 */
void si_on_key(char key,
               key_event_type_t type,
               double held_time,
               si_key_aux_t *aux);

/**
 * create a new player laser originating from the given player
 * should only be created after space-key press and time period elapsed, checked
 * in on_key
 * @param world the scene to add the laser in
 * @param player the player's body pointer
 */
void si_add_player_lasers(scene_t *world, body_t *player);

/**
 * create a new enemy laser originating from the nearest enemy after a certain
 * time period has passed. The nearest enemy is defined as the enemy in the
 * closest column to the player.
 * @param world the scene to add the laser in
 * @param player the player's body pointer, used to calculate the nearest enemy
 */
void si_add_enemy_lasers(scene_t *world, body_t *player);

/**
 * creates destructive forces between the given laser and all the bodies passed
 * in as a list helper function to si_add_enemy_lasers and si_add_player_lasers
 * @param world the scene to add these in
 * @param bodies the list of bodies which can be deleted by this laser
 * @param laser the laser to be coupled with deleting these bodies
 * @param tag the tag of object that should be deleted upon collision with this
 * laser
 */
void si_add_destructive_forces_lasers(scene_t *world,
                                      list_t *bodies,
                                      body_t *laser,
                                      info_e tag);

/**
 * calculates the nearest enemy to the player such that the bottom most player
 * in the nearest column to the player is returned (x > y, not centroid
 * distance)
 * @param world the scene to calculate this in
 * @param player the player's body pointer
 */
body_t *si_closest_enemy(scene_t *world, body_t *player);

// Function Definitions

body_t *si_enemy_init(vector_t initial_position) {
    list_t *enemy_points = list_init((ENEMY_RESOLUTION + 1), free);

    double total_angle = M_PI / ENEMY_RESOLUTION;

    // initializes all of the vectors in the circular part of the enemy,
    // centered at (0,0)
    vector_t *p = NULL;
    for (size_t i = 0; i < ENEMY_RESOLUTION; i++) {
        p = malloc(sizeof(vector_t));
        assert(p != NULL);
        p->x = ENEMY_SIZE / 2.0;
        p->y = 0;
        double curr_angle = i * total_angle;
        *p = vec_rotate(*p, curr_angle);
        list_add(enemy_points, p);
    }

    // add the point to the enemy ship (still centered at (0,0))
    vector_t *tip = malloc(sizeof(vector_t));
    tip->x = 0;
    tip->y = -(ENEMY_SIZE / 4.0);
    list_add(enemy_points, tip);

    enum info *curr_info = malloc(sizeof(int));
    *curr_info = ENEMY_INFO;
    body_t *enemy = body_init_with_info(enemy_points,
                                        ENEMY_MASS,
                                        ENEMY_COLOR,
                                        curr_info,
                                        free);

    body_set_centroid(enemy, initial_position);

    return enemy;
}

body_t *si_player_init(vector_t initial_position) {
    enum info *curr_info = malloc(sizeof(int));
    *curr_info = PLAYER_INFO;
    return make_triangle(PLAYER_COLOR,
                         initial_position,
                         PLAYER_SIZE,
                         PLAYER_MASS,
                         curr_info,
                         free);
}

body_t *si_init_laser(rgb_color_t color,
                      vector_t initial_position,
                      void *laser_info,
                      free_func_t freer) {
    return make_rectangle(color,
                          initial_position,
                          LASER_SIZE,
                          LASER_SIZE / 2.0,
                          LASER_MASS,
                          laser_info,
                          freer);
}

body_t *si_init_laser_enemy(vector_t initial_position) {
    enum info *curr_info = malloc(sizeof(int));
    *curr_info = ENEMY_LASER_INFO;
    return si_init_laser(ENEMY_LASER_COLOR, initial_position, curr_info, free);
}

body_t *si_init_laser_player(vector_t initial_position) {
    enum info *curr_info = malloc(sizeof(int));
    *curr_info = PLAYER_LASER_INFO;
    return si_init_laser(PLAYER_LASER_COLOR, initial_position, curr_info, free);
}

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

void si_world_init(scene_t *si_world, double enemy_rows) {
    scene_set_dims(si_world, MIN, MAX);
    // add the player
    vector_t player_init_position
        = {.x = PLAYER_SIZE, .y = PLAYER_SIZE / 100.0};
    body_t *player = si_player_init(player_init_position);
    scene_add_body(si_world, player); // Position 0 in scene bodies.

    info_e *wall_info = malloc(sizeof(info_e));
    *wall_info = WALL_INFO;
    body_t *left_wall
        = make_rectangle(WALL_COLOR,
                         (vector_t){-WALL_THICKNESS / 2, MAX_Y / 2},
                         MAX_Y,
                         WALL_THICKNESS,
                         INFINITY,
                         wall_info,
                         free);
    wall_info = malloc(sizeof(info_e));
    *wall_info = WALL_INFO;
    body_t *right_wall
        = make_rectangle(WALL_COLOR,
                         (vector_t){WALL_THICKNESS / 2 + MAX_X, MAX_Y / 2},
                         MAX_Y,
                         WALL_THICKNESS,
                         INFINITY,
                         wall_info,
                         free);
    wall_info = malloc(sizeof(info_e));
    *wall_info = WALL_INFO;
    body_t *top_wall
        = make_rectangle(WALL_COLOR,
                         (vector_t){MAX_X / 2, WALL_THICKNESS / 2 + MAX_Y},
                         WALL_THICKNESS,
                         MAX_Y,
                         INFINITY,
                         wall_info,
                         free);
    wall_info = malloc(sizeof(info_e));
    *wall_info = WALL_INFO;
    body_t *bottom_wall
        = make_rectangle(WALL_COLOR,
                         (vector_t){MAX_X / 2, -WALL_THICKNESS / 2},
                         WALL_THICKNESS,
                         MAX_Y,
                         INFINITY,
                         wall_info,
                         free);
    scene_add_body(si_world, left_wall);   // Position 1 in scene bodies.
    scene_add_body(si_world, right_wall);  // Position 2.
    scene_add_body(si_world, top_wall);    // Position 3.
    scene_add_body(si_world, bottom_wall); // Position 4.

    si_create_wall_collision(si_world, player, 0);

    // calculate the num of enemies that can fit in a row based on effective
    // size
    double wid_height = ENEMY_SIZE + BUFFER;
    int num_in_a_row = (int)(MAX_X - MIN_X) / wid_height;

    // add all the enemies to the screen
    for (int row = 0; row < enemy_rows; row++) {
        for (int position_in_row = 0; position_in_row < num_in_a_row;
             position_in_row++) {
            vector_t initial_position
                = {.x = BUFFER_CENTER + ENEMY_SIZE / 2.0
                        + (position_in_row * wid_height),
                   .y = (MAX_Y - ENEMY_SIZE / 2.0) - (row * wid_height)};
            body_t *curr_enemy = si_enemy_init(initial_position);
            scene_add_body(si_world, curr_enemy);
            body_set_velocity(curr_enemy, vec_multiply(ENEMY_SPEED, e1));
            si_create_wall_collision(si_world, curr_enemy, 0);
            create_destructive_collision(si_world, curr_enemy, player);
        }
    }
}

bool end_game(scene_t *world) {
    // if there is no player, end the game
    if (si_get_player(world) == NULL) {
        return true;
    } else {
        for (size_t i = 0; i < scene_bodies(world); i++) {
            body_t *curr_body = scene_get_body(world, i);
            if (*(info_e *)body_get_info(curr_body) == ENEMY_INFO) {
                // if there is a player and at least one enemy, the game is not
                // over
                return false;
            }
        }
    }
    // there are no more enemies to hit, the player has won
    return true;
}

void si_create_wall_collision(scene_t *scene, body_t *body, int n_consts, ...) {
    va_list const_list;
    va_start(const_list, n_consts);

    aux_t *aux = NULL;

    list_t *bodies = scene_get_bodies(scene);

    force_creator_t si_force_creator_function = NULL;
    switch (*((info_e *)body_get_info(body))) {
    case ENEMY_INFO:
        aux = aux_init(n_consts, 1 + 4);

        aux_add_body(aux, body);
        // Add left, right, bottom walls.
        aux_add_body(aux, list_get(bodies, 1));
        aux_add_body(aux, list_get(bodies, 2));
        aux_add_body(aux, list_get(bodies, 4));
        // add player
        aux_add_body(aux, list_get(bodies, 0));

        si_force_creator_function
            = (force_creator_t)si_force_creator_enemy_wall_collision;

        break;
    case PLAYER_INFO:
        aux = aux_init(n_consts, 1 + 2);

        aux_add_body(aux, body);
        // Add left and right walls.
        aux_add_body(aux, list_get(bodies, 1));
        aux_add_body(aux, list_get(bodies, 2));

        si_force_creator_function
            = (force_creator_t)si_force_creator_player_wall_collision;

        break;
    case PLAYER_LASER_INFO:
        aux = aux_init(n_consts, 1 + 1);

        aux_add_body(aux, body);
        // Add the top wall.
        aux_add_body(aux, list_get(bodies, 3));

        si_force_creator_function
            = (force_creator_t)si_force_creator_laser_wall_collision;

        break;
    case ENEMY_LASER_INFO:
        aux = aux_init(n_consts, 1 + 1);

        aux_add_body(aux, body);
        // Add the bottom wall.
        aux_add_body(aux, list_get(bodies, 4));

        si_force_creator_function
            = (force_creator_t)si_force_creator_laser_wall_collision;

        break;
    default:
        break;
    }

    scene_add_bodies_force_creator(scene,
                                   (force_creator_t)si_force_creator_function,
                                   aux,
                                   list_copy(aux->bodies, NULL),
                                   (free_func_t)aux_free);

    va_end(const_list);
}

void si_force_creator_enemy_wall_collision(aux_t *aux) {
    body_t *enemy = list_get(aux->bodies, 0);
    body_t *l_wall = list_get(aux->bodies, 1);
    body_t *r_wall = list_get(aux->bodies, 2);
    body_t *b_wall = list_get(aux->bodies, 3);

    list_t *enemy_shape = body_get_shape(enemy);
    list_t *l_wall_shape = body_get_shape(l_wall);
    list_t *r_wall_shape = body_get_shape(r_wall);
    list_t *b_wall_shape = body_get_shape(b_wall);

    if (find_collision(enemy_shape, b_wall_shape)) {
        body_remove(list_get(aux->bodies, 4));
    } else if (find_collision(enemy_shape, l_wall_shape)) {
        // move the enemy down
        body_translate(enemy,
                       vec_multiply((-NUM_ROWS) * (ENEMY_SIZE + BUFFER), e2));
        // move the enemy away from the edge
        vector_t reset_x
            = {.x = MIN_X + ENEMY_SIZE / 2.0 + 1, .y = body_get_centroid(enemy).y};
        body_set_centroid(enemy, reset_x);
        // switch directions of the enemy's movement
        body_set_velocity(enemy, vec_multiply(-1, body_get_velocity(enemy)));
    } else if (find_collision(enemy_shape, r_wall_shape)) {
        // move the enemy down
        body_translate(enemy,
                       vec_multiply((-NUM_ROWS) * (ENEMY_SIZE + BUFFER), e2));
        // move the enemy away from the edge
        vector_t reset_x
            = {.x = MAX_X - ENEMY_SIZE / 2.0 - 1, .y = body_get_centroid(enemy).y};
        body_set_centroid(enemy, reset_x);
        // switch directions of the enemy's movement
        body_set_velocity(enemy, vec_multiply(-1, body_get_velocity(enemy)));
    }
    list_free(enemy_shape);
    list_free(l_wall_shape);
    list_free(r_wall_shape);
    list_free(b_wall_shape);
}

void si_force_creator_player_wall_collision(aux_t *aux) {
    body_t *player = list_get(aux->bodies, 0);
    body_t *l_wall = list_get(aux->bodies, 1);
    body_t *r_wall = list_get(aux->bodies, 2);

    list_t *player_shape = body_get_shape(player);
    list_t *l_wall_shape = body_get_shape(l_wall);
    list_t *r_wall_shape = body_get_shape(r_wall);

    if (find_collision(player_shape, l_wall_shape)) {
        body_translate(player, e1);
        body_set_velocity(player, VEC_ZERO);

    } else if (find_collision(player_shape, r_wall_shape)) {
        body_translate(player, vec_multiply(-1, e1));
        body_set_velocity(player, VEC_ZERO);
    }
    list_free(player_shape);
    list_free(l_wall_shape);
    list_free(r_wall_shape);
}
void si_force_creator_laser_wall_collision(aux_t *aux) {
    body_t *laser = list_get(aux->bodies, 0);
    body_t *wall = list_get(aux->bodies, 1);

    list_t *laser_shape = body_get_shape(laser);
    list_t *wall_shape = body_get_shape(wall);

    if (find_collision(laser_shape, wall_shape)) {
        body_remove(laser);
    }
    list_free(laser_shape);
    list_free(wall_shape);
}

si_key_aux_t *si_key_aux_init(scene_t *scene) {
    si_key_aux_t *aux = malloc(sizeof(si_key_aux_t));
    aux->scene = scene;
    aux->time_since_last_shoot = 0;
    aux->player = si_get_player(scene);
    return aux;
}

body_t *si_get_player(scene_t *scene) {
    body_t *no_hit = scene_get_body(scene, 0);
    if (*(info_e *)body_get_info(no_hit) == PLAYER_INFO) {
        return no_hit;
    }
    return NULL;
}

void si_key_aux_tick(si_key_aux_t *aux, double dt) {
    aux->time_since_last_shoot += dt;
}

void si_on_key(char key,
               key_event_type_t type,
               double held_time,
               si_key_aux_t *aux) {
    scene_t *scene = aux->scene;
    double time_since_last_shoot = aux->time_since_last_shoot;
    body_t *player = aux->player;
    switch (key) {
    case ' ': // SPACE
        if (type == KEY_PRESSED && time_since_last_shoot > PLAYER_LASER_DELAY) {
            si_add_player_lasers(scene, player);
            aux->time_since_last_shoot = 0;
        }
        break;
    case LEFT_ARROW:
        switch (type) {
        case KEY_PRESSED: // Move left.
            body_set_velocity(player, vec_multiply(-PLAYER_SPEED, e1));
            break;
        case KEY_RELEASED: // Stop moving.
            body_set_velocity(player, VEC_ZERO);
            break;
        }
        break;
    case RIGHT_ARROW:
        switch (type) {
        case KEY_PRESSED: // Move right.
            body_set_velocity(player, vec_multiply(PLAYER_SPEED, e1));
            break;
        case KEY_RELEASED: // Stop moving.
            body_set_velocity(player, VEC_ZERO);
            break;
        }
        break;
    }
}

void si_add_player_lasers(scene_t *world, body_t *player) {
    vector_t center
        = vec_add(body_get_centroid(player), vec_multiply(PLAYER_SIZE, e2));
    body_t *new_laser = si_init_laser_player(center);
    // player shoots up (+y)
    body_set_velocity(new_laser, vec_multiply(LASER_SPEED, e2));
    scene_add_body(world, new_laser);
    si_add_destructive_forces_lasers(world,
                                     scene_get_bodies(world),
                                     new_laser,
                                     ENEMY_INFO);
    si_create_wall_collision(world, new_laser, 0);
}

void si_add_enemy_lasers(scene_t *world, body_t *player) {
    body_t *nearest_enemy = si_closest_enemy(world, player);
    if (nearest_enemy != NULL) {
        vector_t center = body_get_centroid(nearest_enemy);
        body_t *new_laser = si_init_laser_enemy(center);
        // enemy shoots down (-y)
        body_set_velocity(new_laser, vec_multiply(-LASER_SPEED, e2));
        scene_add_body(world, new_laser);
        si_add_destructive_forces_lasers(world,
                                         scene_get_bodies(world),
                                         new_laser,
                                         PLAYER_INFO);
        si_create_wall_collision(world, new_laser, 0);
    }
}

void si_add_destructive_forces_lasers(scene_t *world,
                                      list_t *bodies,
                                      body_t *laser,
                                      info_e tag) {
    for (size_t i = 0; i < list_size(bodies); i++) {
        body_t *curr_body = list_get(bodies, i);
        // add the destructive_collision ONLY if the body is of the right type
        if (*(info_e *)body_get_info(curr_body) == tag) {
            create_destructive_collision(world, laser, curr_body);
        }
    }
}

body_t *si_closest_enemy(scene_t *world, body_t *player) {
    vector_t player_center = body_get_centroid(player);
    body_t *closest = NULL;
    double nearest_distance = INFINITY;
    double nearest_height = INFINITY;

    for (size_t i = 4; i < scene_bodies(world); i++) {
        body_t *curr_body = scene_get_body(world, i);
        // only check the body if it is an enemy ship
        if (*(info_e *)body_get_info(curr_body) == ENEMY_INFO) {
            vector_t curr_center = body_get_centroid(curr_body);
            double distance = fabs(player_center.x - curr_center.x);
            double height = fabs(player_center.y - curr_center.y);
            // to calculate nearest column only (not centroid distance), use
            // only x components
            if (distance < nearest_distance) {
                nearest_distance = distance;
                nearest_height = height;
                closest = curr_body;
            }
            // if the x distance is equal, the ships are in the same column.
            // calculate by height
            else if ((distance < nearest_distance + BUFFER_CENTER)
                     && (distance > nearest_distance - BUFFER_CENTER)) {
                if (height < nearest_height) {
                    nearest_distance = distance;
                    nearest_height = height;
                    closest = curr_body;
                }
            }
        }
    }
    return closest;
}

int main(int argc, char **argv) {

    scene_t *si_world = scene_init();
    si_world_init(si_world, NUM_ROWS);
    si_key_aux_t *key_aux = si_key_aux_init(si_world);

    double dt = 0.0;
    double total_time = 0.0;
    double time_since_last_enemy_laser = 0.0;

    sdl_init(MIN, MAX);
    sdl_on_key((key_handler_t)si_on_key, key_aux);

    while (!sdl_is_done() && !end_game(si_world)) {
        dt = time_since_last_tick();
        if (time_since_last_enemy_laser > ENEMY_LASER_DELAY) {
            si_add_enemy_lasers(si_world, si_get_player(si_world));
            time_since_last_enemy_laser = 0;
        }
        scene_tick(si_world, dt);
        sdl_render_scene(si_world);
        time_since_last_enemy_laser += dt;
        total_time += dt;
        si_key_aux_tick(key_aux, dt);
    }

    free(key_aux);
    scene_free(si_world);

    return 0;
}
