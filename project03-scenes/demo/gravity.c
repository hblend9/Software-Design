#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "sdl_wrapper.h"
#include "list.h"
#include "polygon.h"
#include "star.h"
#include "math.h"

const vector_t MIN = {.x = 0, .y = 0};
const vector_t MAX = {.x = 1000, .y = 500};
const vector_t CENTER = {.x = 0, .y = 500};
const vector_t VELOCITY = {.x = 175, .y = 0};

const double RADIUS = 30.0;
const int NUM_STARS = 2;
const int NUM_VERTICES = 4;
const double ROT_SPEED = 2.5;
const double START_TIME = 1.0;
const double CLOCK_START = 2.0;
const double CLOCK_RESET = 2.0;
const double CLOCK_NEW_STAR = 3.5;
const double GRAVITY = 1500;

void add_star(list_t *star_arr, star_t *star){
    list_add(star_arr, star);
}

/** Checks if star reaches bottom.
 * @param vertices list of vertices of the star
 * @param velocity the velocity of the star
 * If it reaches bottom, returns true. Else, returns false.
 */
bool reaches_bottom(star_t *star){
    int num_vertices = list_size(star_get_vertices(star));
    for (int i = 0; i < num_vertices; i++){
        double y = ((vector_t*)list_get(star_get_vertices(star), i))->y;
        if (y <= MIN.y && star_get_velocity(star)->y < 0){
            return true;
        }
    }
    return false;
}

/** Checks if star is out of bounds.
 * If it is out of bounds, returns . Else, returns 0.
 */
bool star_out_of_bounds(star_t *star){
    int num_vertices = list_size(star_get_vertices(star));
    for (int i = 0; i < num_vertices; i++){
        double x = ((vector_t*)list_get(star_get_vertices(star), i))->x;
        if (x <= MAX.x){
            return false;
        }
    }
    return true;
}

/** Removes all stars that are out of bounds.
 * @param star_arr array of stars to check
*/
void remove_out_of_bounds_stars(list_t *star_arr){
   star_t *curr = list_get(star_arr, 0);
   if (star_out_of_bounds(curr)){
        list_remove(star_arr, 0);
    }
}

/** Computes the new position of one star.
 * @param star star
 * @param dt time since last update
 */
void compute_star_position(star_t *star, double dt){
    vector_t v = {.x = star_get_velocity(star)->x, .y = star_get_velocity(star)->y};

    if (reaches_bottom(star)){
        star_get_velocity(star)->y *= -1.0 * star_get_elasticity(star);
    }

    star_get_velocity(star)->y -= GRAVITY * dt;
   
    polygon_translate(star_get_vertices(star), vec_multiply(dt, v));
    polygon_rotate(star_get_vertices(star), ROT_SPEED * dt, polygon_centroid(star_get_vertices(star)));
}

/** Computes new positions of all the stars.
 * @param star_arr array of stars
 * @param dt time since last update
*/
void compute_new_positions(list_t *star_arr, double dt){
    int size = list_size(star_arr);
    for (int i = 0; i < size; i++){
        star_t *curr = list_get(star_arr, i);
        compute_star_position(curr, dt);
    }
}

/** Draws all the stars.
 * @param star_arr array of stars
*/
void draw_stars(list_t *star_arr){
    int size = list_size(star_arr);
    for (int i = 0; i < size; i++){
        star_t *curr = list_get(star_arr, i);
        sdl_draw_polygon((list_t*)star_get_vertices(curr), star_get_color(curr));
    }
}

/** Updates graphics for all stars.
 * @param star_arr array of stars
*/
void update_window(list_t *star_arr){
    sdl_clear();
    draw_stars(star_arr);
    sdl_show();
}


int main(){
    list_t *star_arr = list_init(NUM_STARS, free); 
    double clock = 0;

    int num_vertices = NUM_VERTICES;
    sdl_init(MIN, MAX); 
   
    while(!sdl_is_done()){
        double dt = time_since_last_tick();
        clock += dt;

        if (clock < START_TIME){
            vector_t *v = malloc(sizeof(vector_t)); //defines initial velocity of first star
            v->x = VELOCITY.x;
            v->y = VELOCITY.y;

            star_t *init_star = star_init(CENTER, RADIUS, num_vertices, v);
            add_star(star_arr, init_star); 
            clock = CLOCK_RESET;
        }

        else if (clock > CLOCK_NEW_STAR){ //every 1.5 seconds
            clock = CLOCK_RESET;
            num_vertices += 2;
           
            vector_t *v_0 = malloc(sizeof(vector_t)); 
            v_0->x = VELOCITY.x;
            v_0->y = VELOCITY.y;

            star_t *star = star_init(CENTER, RADIUS, num_vertices, v_0);
            add_star(star_arr, star);
        }
       
        compute_new_positions(star_arr, dt);
        remove_out_of_bounds_stars(star_arr);
        update_window(star_arr);
    }

    list_free(star_arr);
}