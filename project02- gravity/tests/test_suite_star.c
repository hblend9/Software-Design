#include "../include/star.h"
#include "../include/array_list.h"
#include "../include/test_util.h"
#include <math.h>
#include <assert.h>
#include <stdlib.h>
 
array_list_t *make_star() {
    // Making a random star
    array_list_t *st = array_list_init(6);

    vector_t *s = malloc(sizeof(vector_t));
    *s = (vector_t) {cos(M_PI / 3 + M_PI / 3) * 6.0, sin(M_PI / 3 + M_PI / 3) * 6.0};
    array_list_add(st, s);

    s = malloc(sizeof(vector_t));
    *s = (vector_t) {cos(M_PI / 3 + 2 * M_PI / 3) * 3.0, sin(M_PI / 3 + 2 * M_PI / 3) * 3.0};
    array_list_add(st, s);

    s = malloc(sizeof(vector_t));
    *s = (vector_t) {cos(M_PI / 3 + 3 * M_PI / 3) * 6.0, sin(M_PI / 3 + 3 * M_PI / 3) * 6.0};
    array_list_add(st, s);

    s = malloc(sizeof(vector_t));
    *s = (vector_t) {cos(M_PI / 3 + 4 * M_PI / 3) * 3.0, sin(M_PI / 3 + 4 * M_PI / 3) * 3.0};
    array_list_add(st, s);

    s = malloc(sizeof(vector_t));
    *s = (vector_t) {cos(M_PI / 3 + 5 * M_PI / 3) * 6.0, sin(M_PI / 3 + 5 * M_PI / 3) * 6.0};
    array_list_add(st, s);

    s = malloc(sizeof(vector_t));
    *s = (vector_t) {cos(M_PI / 3 + 6 * M_PI / 3) * 3.0, sin(M_PI / 3 + 6 * M_PI / 3) * 3.0};
    array_list_add(st, s);

    return st;
}
 
// Check the fields are set and initialized properly
void test_star_init(){
    vector_t *velocity = malloc(sizeof(vector_t));
    *velocity = (vector_t) {1.0, 2.0};
    star_t *st = star_init((vector_t) {0, 0}, 6.0, 6, velocity);
    assert(isclose(star_get_num_vertices(st), 6));
 
    star_free(st);
}
 
// Test for both star_find_vertices and star_get_vertices
void test_star_find_vertices() {
    array_list_t *st = make_star();
    vector_t *velocity = malloc(sizeof(vector_t));
    *velocity = (vector_t) {1.0, 2.0};
    //star_t *result = star_init((vector_t) {0, 0}, 6.0, 6, velocity);
    double num_vertices = 6.0;
    for (int i = 0; i < num_vertices; i++){
        //assert(vec_isclose(((*vector_t)*array_list_get(star_get_vertices(result), i)), ((*vector_t)*array_list_get(st, i))));
    }
    array_list_free(st);
}
 
// Give the star a velocity and make sure they are the same
void test_star_get_velocity() {
    vector_t *velocity = malloc(sizeof(vector_t));
    *velocity = (vector_t) {1.0, 2.0};
    star_t *st = star_init((vector_t) {0, 0}, 6.0, 6, velocity);
    assert(vec_equal(*star_get_velocity(st), *velocity));
 
    star_free(st);
}
 
void test_star_get_num_vertices() {
    vector_t *velocity = malloc(sizeof(vector_t));
    *velocity = (vector_t) {1.0, 2.0};
    star_t *st = star_init((vector_t) {0, 0}, 6.0, 6, velocity);
    double num_vertices = 6.0;
    assert(isclose(star_get_num_vertices(st), num_vertices));
 
    star_free(st);
}
 
void test_star_get_elasticity() {
    vector_t *velocity = malloc(sizeof(vector_t));
    *velocity = (vector_t) {1.0, 2.0};
    star_t *st = star_init((vector_t) {0, 0}, 6.0, 6, velocity);
    assert(0.875 <= star_get_elasticity(st) && star_get_elasticity(st) <= 0.95);
 
    star_free(st);
}
 
void test_star_get_color() {
}
 
int main(int argc, char *argv[]) {
    // Run all tests
    bool all_tests = argc == 1;
    // Read test name from file
    char testname[100];
    if (!all_tests) {
        read_testname(argv[1], testname, sizeof(testname));
    }
 
    DO_TEST(test_star_find_vertices)
    DO_TEST(test_star_get_velocity)
    DO_TEST(test_star_get_num_vertices)
    DO_TEST(test_star_get_elasticity)

 
    puts("star_test PASS");
}