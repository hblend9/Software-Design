#include <stdio.h>
#include <stdlib.h>
#include "sdl_wrapper.h"
#include "polygon.h"
#include "list.h"
#include "math.h"

const vector_t MIN_BOUNCE = {.x = 0, .y = 0};
const vector_t MAX_BOUNCE = {.x = 1000, .y = 500};
const vector_t CENTER_BOUNCE = {.x = 500, .y = 250};
const vector_t VELOCITY = {.x = 1000, .y = 1000};
const double PENTAGON_RATIO = 2.61803398875;
const double RADIUS_BOUNCES = 50.0;
const double ROT_SPEED = 2.5;
const double COLOR_R = 0.635;
const double COLOR_G = 0.839;
const double COLOR_B = 0.976;

list_t *make_polygon(vector_t CENTER_BOUNCE, double RADIUS_BOUNCEs){
    list_t *star_vertices = list_init(10, (free_func_t) free);

    double start_angle = M_PI / 5.0;
    double change_angle = M_PI / 5.0;

    for (int i = 0; i < 10; i++){
        vector_t *vertex = malloc(sizeof(vector_t));
        if (i % 2 == 0){
            vertex->x = cos(start_angle + (i + 1) * change_angle) * RADIUS_BOUNCES + CENTER_BOUNCE.x; 
            vertex->y = sin(start_angle + (i + 1) * change_angle) * RADIUS_BOUNCES + CENTER_BOUNCE.y;
        }
        else{
            vertex->x = cos(start_angle + (i + 1) * change_angle) * (RADIUS_BOUNCES / PENTAGON_RATIO) + CENTER_BOUNCE.x;
            vertex->y = sin(start_angle + (i + 1) * change_angle) * (RADIUS_BOUNCES / PENTAGON_RATIO) + CENTER_BOUNCE.y;
        }
        list_add(star_vertices, vertex);   
    }

    return star_vertices;
}

char change_velocity_collision(list_t *vertices, vector_t velocity){
    for (int i = 0; i < 10; i++){
        double x = ((vector_t*)list_get(vertices, i))->x;
        double y = ((vector_t*)list_get(vertices, i))->y;
        if ((x <= MIN_BOUNCE.x && velocity.x < 0) || (x >= MAX_BOUNCE.x && velocity.x > 0)){
            return 'x';
        }
        else if ((y <= MIN_BOUNCE.y && velocity.y < 0) || (y >= MAX_BOUNCE.y && velocity.y > 0)){
            return 'y';
        }
    }
    return 'n';
}

vector_t compute_new_positions(list_t *vertices, vector_t velocity, double dt){
    if (change_velocity_collision(vertices, velocity) == 'x'){
        velocity.x *= -1.0;
    }
    if (change_velocity_collision(vertices, velocity) == 'y'){
        velocity.y *= -1.0;
    }
    for (int i = 0; i < 10; i++){
        vector_t *prev = list_get(vertices, i);

        prev->x += velocity.x * dt;
        prev->y += velocity.y * dt;

        list_replace(vertices, i, prev);
    }
    polygon_rotate(vertices, ROT_SPEED * dt, polygon_centroid(vertices));
    return velocity;
}

void update_window(list_t *vertices){
    sdl_clear();
    //sdl_draw_polygon(vertices, COLOR_R, COLOR_G, COLOR_B);
    sdl_show();
}

int main(){
    list_t *star = make_polygon(CENTER_BOUNCE, RADIUS_BOUNCES);
    sdl_init(MIN_BOUNCE, MAX_BOUNCE);
    vector_t speed = VELOCITY;

    while(!sdl_is_done()){
        double dt = time_since_last_tick();
        speed = compute_new_positions(star, speed, dt);
        update_window(star);
    }
    list_free(star);
}