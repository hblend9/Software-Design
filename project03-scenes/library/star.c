#include <stdlib.h>
#include <stdio.h>
#include "star.h"
#include "list.h"
#include "math.h"
#include "color.h"

const double ELASTICITY_START = 0.875;
const double ELASTICITY_END = 0.075;

typedef struct star{
    vector_t center;
    double radius;
    int num_vertices;

    list_t *vertices;
    rgb_color_t color;

    vector_t *velocity;
    double elasticity;
}star_t;

star_t *star_init(vector_t center, double radius, int num_vertices, vector_t *velocity){
    star_t *my_star = malloc(sizeof(star_t));

    my_star->center = center;
    my_star->radius = radius;
    my_star->num_vertices = num_vertices;

    my_star->vertices = star_find_vertices(center, radius, num_vertices);
    my_star->color = rgb_color_init();
    my_star->velocity = velocity;
    my_star->elasticity = ELASTICITY_START + (float) rand() / (float) (RAND_MAX / ELASTICITY_END);
    return my_star;
}

void star_free(star_t *st){
    list_free(st->vertices);
    free(st);
}

list_t *star_find_vertices(vector_t center, double radius, int num_vertices){
    list_t *star_vertices = list_init(num_vertices, (free_func_t) free);

    double start_angle = M_PI / (num_vertices / 2);
    double change_angle = M_PI / (num_vertices / 2);

    for (int i = 0; i < num_vertices; i++){
        vector_t *vertex = malloc(sizeof(vector_t));
        if (i % 2 == 0){
            vertex->x = cos(start_angle + (i + 1) * change_angle) * radius + center.x; 
            vertex->y = sin(start_angle + (i + 1) * change_angle) * radius + center.y;
        }
        else{
            vertex->x = cos(start_angle + (i + 1) * change_angle) * (radius / 2) + center.x;
            vertex->y = sin(start_angle + (i + 1) * change_angle) * (radius / 2) + center.y;
        }
        list_add(star_vertices, vertex);   
    }
    return star_vertices;
}

list_t *star_get_vertices(star_t *star){
    return star->vertices;
}

rgb_color_t star_get_color(star_t *star){
    return star->color;
}

vector_t *star_get_velocity(star_t *star){
    return star->velocity;
}

int star_get_num_vertices(star_t *star){
    return star->num_vertices;
}

double star_get_elasticity(star_t *star){
    return star->elasticity;
}
