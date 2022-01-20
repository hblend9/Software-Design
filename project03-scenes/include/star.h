#include "list.h"
#include "vector.h"
#include "color.h"

typedef struct star star_t;
 
star_t *star_init(vector_t center, double radius, int num_vertices, vector_t *velocity);
 
void star_free(star_t *st);
 
list_t *star_find_vertices(vector_t center, double radius, int num_vertices);
 
list_t *star_get_vertices(star_t *star);
 
vector_t *star_get_velocity(star_t *star);

rgb_color_t star_get_color(star_t *star);
 
int star_get_num_vertices(star_t *star);
 
double star_get_elasticity(star_t *star);