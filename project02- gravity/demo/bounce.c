#include <stdio.h>
#include <stdlib.h>
#include "sdl_wrapper.h"
#include "polygon.h"
#include "array_list.h"
#include "math.h"

const vector_t MIN = {.x = 0, .y = 0};
const vector_t MAX = {.x = 1000, .y = 500};
const vector_t CENTER = {.x = 500, .y = 250};
const vector_t VELOCITY = {.x = 1000, .y = 1000};
const double PENTAGON_RATIO = 2.61803398875;
const double RADIUS = 50.0;
const double ROT_SPEED = 2.5;
const double COLOR_R = 0.635;
const double COLOR_G = 0.839;
const double COLOR_B = 0.976;

array_list_t *make_polygon(vector_t center, double radius){
   array_list_t *star_vertices = array_list_init(10);

    double start_angle = M_PI / 5.0;
    double change_angle = M_PI / 5.0;

    for (int i = 0; i < 10; i++){
        vector_t *vertex = malloc(sizeof(vector_t));
        if (i % 2 == 0){
            vertex->x = cos(start_angle + (i + 1) * change_angle) * RADIUS + center.x; 
            vertex->y = sin(start_angle + (i + 1) * change_angle) * RADIUS + center.y;
        }
        else{
            vertex->x = cos(start_angle + (i + 1) * change_angle) * (RADIUS / PENTAGON_RATIO) + center.x;
            vertex->y = sin(start_angle + (i + 1) * change_angle) * (RADIUS / PENTAGON_RATIO) + center.y;
        }
        array_list_add(star_vertices, vertex);   
    }
    return star_vertices;
}

char change_velocity_collision(array_list_t *vertices, vector_t velocity){
    for (int i = 0; i < 10; i++){
        double x = ((vector_t*)array_list_get(vertices, i))->x;
        double y = ((vector_t*)array_list_get(vertices, i))->y;
        if ((x <= MIN.x && velocity.x < 0) || (x >= MAX.x && velocity.x > 0)){
            return 'x';
        }
        else if ((y <= MIN.y && velocity.y < 0) || (y >= MAX.y && velocity.y > 0)){
            return 'y';
        }
    }
    return 'n';
}

vector_t compute_new_positions(array_list_t *vertices, vector_t velocity, double dt){
    if (change_velocity_collision(vertices, velocity) == 'x'){
        velocity.x *= -1.0;
    }
    if (change_velocity_collision(vertices, velocity) == 'y'){
        velocity.y *= -1.0;
    }
    for (int i = 0; i < 10; i++){
        vector_t *prev = array_list_get(vertices, i);

        prev->x += velocity.x * dt;
        prev->y += velocity.y * dt;

        array_list_replace(vertices, i, prev);
    }
    polygon_rotate(vertices, ROT_SPEED * dt, polygon_centroid(vertices));
    return velocity;
}

void update_window(array_list_t *vertices){
    sdl_clear();
    sdl_draw_polygon(vertices, COLOR_R, COLOR_G, COLOR_B);
    sdl_show();
}

int main(){
    array_list_t *star = make_polygon(CENTER, RADIUS);
    sdl_init(MIN, MAX);
    vector_t speed = VELOCITY;

    while(!sdl_is_done()){
        double dt = time_since_last_tick();
        speed = compute_new_positions(star, speed, dt);
        update_window(star);
    }
    array_list_free(star);
}