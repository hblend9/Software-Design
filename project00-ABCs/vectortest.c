#include "vector.h"

int main(int argc, char *argv[]) {
    vector_t *vector1 = vec_init(0.0, 0.0);
    // test vec_init
    printf("the x component of your vector is: %f\n", get_x(vector1));
    printf("the y component of your vector is: %f\n", get_y(vector1));
    //test vec_add
    vector_t *vector2 = vec_init(1.0, 1.0);
    vector_t *vector3 = vec_init(2.0, 2.0);
    vector_t *vector4 = vec_add(vector2, vector3);
    printf("the x component of sum of vectors is: %f\n", get_x(vector4));
    printf("the y component of sum of vectors is: %f\n", get_x(vector4));
}