#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "polygon.h"
#include <math.h>

const double CENTROID = 1.0/6.0;

double polygon_area(array_list_t *polygon){
    double area = 0.0;
    int vertices = array_list_size(polygon);
    for (int i = 0; i < vertices; i++)
    {
        double x = ((vector_t*)array_list_get(polygon, i)) -> x;
        double posY = ((vector_t*)array_list_get(polygon, (vertices + i + 1) % vertices)) -> y;
        double negY = ((vector_t*)array_list_get(polygon, (vertices + i - 1) % vertices)) -> y;
        area += (x * posY) - (x * negY);
    }
    area = fabs(0.5 * area);
    return area;
}

vector_t polygon_centroid(array_list_t *polygon){
    double area = polygon_area(polygon);
    double vx = 0.0;
    double vy = 0.0;
    int vertices = array_list_size(polygon);
    for (int i = 0; i < vertices; i++)
    {
        double x0 = ((vector_t*)array_list_get(polygon, i)) -> x;
        double x1 = ((vector_t*)array_list_get(polygon, (vertices + i + 1) % vertices)) -> x;
        double y0 = ((vector_t*)array_list_get(polygon, i)) -> y;
        double y1 = ((vector_t*)array_list_get(polygon, (vertices + i + 1) % vertices)) -> y;

        vx += (x0 + x1) * (x0*y1 - x1*y0);
        vy += (y0 + y1) * (x0*y1 - x1*y0);
    }
    vector_t centroid = {
        .x = CENTROID * (1.0/area) * vx,
        .y = CENTROID * (1.0/area) * vy
    };
    return centroid;
}

void polygon_translate(array_list_t *polygon, vector_t translation){
    int vertices = array_list_size(polygon);
    for (int i = 0; i < vertices; i++)
    {
        double new_x = ((vector_t*)array_list_get(polygon, i)) -> x + translation.x;
        double new_y = ((vector_t*)array_list_get(polygon, i)) -> y + translation.y;

        ((vector_t*)array_list_get(polygon, i)) -> x = new_x;
        ((vector_t*)array_list_get(polygon, i)) -> y = new_y;
    }
}

void polygon_rotate(array_list_t *polygon, double angle, vector_t point){
    vector_t negated_pt = vec_negate(point);
    polygon_translate(polygon, negated_pt);

    int vertices= array_list_size(polygon);

    for (int i = 0; i < vertices; i++)
    {
        vector_t old_vector = {
            .x = ((vector_t*)array_list_get(polygon, i)) -> x,
            .y = ((vector_t*)array_list_get(polygon, i)) -> y
        };
        
        vector_t rotated_vector = vec_rotate(old_vector, angle);

        ((vector_t*)array_list_get(polygon, i)) -> x = rotated_vector.x;
        ((vector_t*)array_list_get(polygon, i)) -> y = rotated_vector.y;
    }
    polygon_translate(polygon, point);
}