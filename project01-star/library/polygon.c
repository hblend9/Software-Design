#include "vec_list.h"
#include "vector.h"
#include "polygon.h"
#include "math.h"
#include <stdio.h>


double polygon_signed_area(vec_list_t *polygon){
    const double CONSTANT = 1.0/2.0;
    size_t vertex_num = vec_list_size(polygon);
        double area = 0.0;
        size_t k = vertex_num - 1;
        for (size_t i = 0; i < vertex_num; i++) {
            vector_t *vk = vec_list_get(polygon, k);
            vector_t * vi = vec_list_get(polygon, i);
            area += (vk->x * vi->y);
            area -= (vi->x * vk->y);
            k = i;
    }
    return area * CONSTANT;
}
 

double polygon_area(vec_list_t *polygon){
    double area = polygon_signed_area(polygon);
    return fabs(area);
}


vector_t polygon_centroid(vec_list_t *polygon){
    double x = 0.0;
    double y = 0.0;
    const double CONSTANT = 1.0/6.0;
    for(size_t i = 0; i < vec_list_size(polygon); i++) {
        double y1 = vec_list_get(polygon, i)->y;
        double y2;
        if (i == vec_list_size(polygon) - 1) {
            y2 = vec_list_get(polygon, 0)->y;
        }
        else {
            y2 = vec_list_get(polygon, i + 1)->y;
        }
        double x1 = vec_list_get(polygon, i)->x;
        double x2;
        if (i == vec_list_size(polygon) - 1) {
            x2 = vec_list_get(polygon, 0)->x;
        }
        else {
            x2 = vec_list_get(polygon, i + 1)->x;
        }
        x += (x1 + x2) * ((x1 * y2) - (x2 * y1));
        y += (y1 + y2) * ((x1 * y2) - (x2 * y1));
    }
    x = (x * (CONSTANT / polygon_area(polygon)));
    y = (y * (CONSTANT / polygon_area(polygon)));
    vector_t centroid = {x, y};
    return centroid;
}



void polygon_translate(vec_list_t *polygon, vector_t translation){
    size_t size = vec_list_size(polygon);
    for (int i = 0; i < (int) size; i++) {
        vector_t *current_point = vec_list_get(polygon, i);
        vector_t translate_point = vec_add(*current_point, translation);
        *current_point = translate_point;
    }
}


void polygon_rotate(vec_list_t *polygon, double angle, vector_t point){
    for (int i = 0; i < (int) vec_list_size(polygon); i++) {
        vector_t *current_point = vec_list_get(polygon, i);
        vector_t translate_point = vec_subtract(*current_point, point);
        vector_t rotated_vec = vec_rotate(translate_point, angle);
        *current_point = vec_add(rotated_vec, point);
    }
}