/** @file vector.h
 *  @brief Function prototypes for a 2D vector.
 *
 *  This contains the prototypes for a very small
 *  amount of 2D vector functionality.
 *
 *  @author Adam Blank (blank)
 *  @bug No known bugs.
 */

#ifndef VECTOR_H
#define VECTOR_H
#endif

typedef struct vector_t { //add vector before vector_t?
    double x;
    double y;
} vector_t;

vector_t *vec_init(double x, double y) {
    vector_t *v = malloc(sizeof(vector_t));
    v->x = x;
    v->y = y;
    printf("%f, %f\n", v->x, v->y);
    free(v);
}

void vec_free(vector_t *vec) {
    char *result = malloc(sizeof(*vec));
    free(vec);
}

vector_t *vec_add(vector_t *v1, vector_t *v2) {
    double v3x = v1->x + v2->x;
    double v3y = v1->y + v2->y;
    return vec_init(v3x, v3y);
}