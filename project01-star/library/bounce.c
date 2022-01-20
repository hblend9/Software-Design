#include "sdl_wrapper.h"
#include "polygon.h"
#include <vec_list.h>
#include <vector.h>
#include <math.h>
#include <test_util.h>
#include <stdio.h>
#include <stdlib.h>

const vector_t min = {0, 0};
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const vector_t max = {WINDOW_WIDTH, WINDOW_HEIGHT};

const int rad = 60;
const double rotate = 6;
const int corners = 5;

vec_list_t* make_star(int coords, int rad, vector_t middle){
    vec_list_t *star_shape = vec_list_init(2 * coords);

    double theta = 2 * M_PI / coords;

    vector_t v;
    v.x = middle.x;
    v.y = middle.y + rad;

    double r_inner = rad / 3; //inner star radius

    vector_t v_inner;

    double inner_theta = (M_PI / 2) + (theta / 2); //inner angle

    v_inner.x = middle.x + (r_inner * sin(inner_theta));
    v_inner.y = middle.y + (r_inner * cos(inner_theta));

    v_inner = vec_add(v_inner, middle); //v_inner is now the inside point

    for(size_t i = 0; i < corners; i++){
        vector_t *outer_star = malloc(sizeof(vector_t));
        double angle_used_to_rot = theta * i;

        vector_t *inner_star = malloc(sizeof(vector_t));
        *inner_star = vec_subtract(v_inner, middle);
        *inner_star = vec_rotate(*inner_star, angle_used_to_rot);
        *inner_star = vec_subtract(*inner_star, middle);

        *outer_star = vec_subtract(v, middle);
        *outer_star = vec_rotate(*outer_star, angle_used_to_rot);
        *outer_star = vec_subtract(*outer_star, middle);

        vec_list_add(star_shape, inner_star);
        vec_list_add(star_shape, outer_star);
    }
    return star_shape;
}

vector_t new_position(vector_t position, double dt, double rad, vec_list_t *shape){
    vector_t mid = polygon_centroid(shape);
    vector_t change_in_position = vec_multiply(dt, position);

    double change_in_x = change_in_position.x;
    double change_in_y = change_in_position.y;
    double sum_x = mid.x + change_in_position.x;
    double sum_y = mid.x + change_in_position.y;

    if(change_in_position.x > 0 && sum_x + rad >= WINDOW_WIDTH){
        position.x *= -1;
        change_in_x = WINDOW_WIDTH - mid.x - rad - change_in_position.x;
    }
    else if(change_in_position.x < 0 && sum_x - rad <= 0){
        position.x *= -1;
        change_in_x = rad - mid.x - change_in_position.x;
    }
    if(change_in_position.y > 0 && sum_y + rad >= WINDOW_HEIGHT){
        position.y *= -1;
        change_in_y = WINDOW_HEIGHT - mid.y - rad - change_in_position.y;
    }
    else if(change_in_position.y < 0 && sum_y - rad <= 0){
        position.y *= -1;
        change_in_y = rad - mid.y - change_in_position.y;
    }
    vector_t tot_change;
    tot_change.x = change_in_x;
    tot_change.y = change_in_y;

    polygon_translate(shape, tot_change);
    polygon_rotate(shape, rotate * dt, mid);

    return position;
}

int main(){
    const int x_vel = 20;
    const int y_vel = 20;

    vector_t second_position;
    second_position.x = x_vel;
    second_position.y = y_vel;

    vector_t center_point;
    center_point.x = WINDOW_WIDTH / 2;
    center_point.y = WINDOW_HEIGHT / 2;

    vec_list_t *shape = make_star(corners, rad, center_point);

    sdl_init(min, max);

    const int red = 0;
    const int green = 1;
    const int blue = 1;

    while(!sdl_is_done()){
        double dt = time_since_last_tick();
        sdl_clear(); //when void parameter just call w ()
        second_position = new_position(second_position, dt, rad, shape);
        sdl_draw_polygon(shape, red, green, blue);
        sdl_show();
    }
    vec_list_free(shape);

    return 0;
}