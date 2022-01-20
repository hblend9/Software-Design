#include "color.h"
#include "forces.h"
#include "test_util.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

// (Print test data to stderr in csv format.)
const bool VERBOSE_TESTS = false;
// (Bypass test assertions; for testing the tests.)
const bool DONT_FAIL = false;
const rgb_color_t UNCOLOR = {0, 0, 0};

/** UTILITIES **/

list_t *make_shape() {
    list_t *shape = list_init(4, free);
    vector_t *v = malloc(sizeof(*v));
    *v = (vector_t){-1, -1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (vector_t){+1, -1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (vector_t){+1, +1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (vector_t){-1, +1};
    list_add(shape, v);
    return shape;
}

body_t *make_anchor() {
    return body_init(make_shape(), INFINITY, UNCOLOR);
}

body_t *make_body(double mass) {
    return body_init(make_shape(), mass, UNCOLOR);
}

vector_t weighted_vector_mean(vector_t v1, double w1, vector_t v2, double w2) {
    double w = w1 + w2;
    return vec_add(vec_multiply(w1 / w, v1), vec_multiply(w2 / w, v2));
}

vector_t velocity_cm(body_t *b1, body_t *b2) {
    double m1 = body_get_mass(b1);
    double m2 = body_get_mass(b2);
    vector_t v1 = body_get_velocity(b1);
    vector_t v2 = body_get_velocity(b2);
    return weighted_vector_mean(v1, m1, v2, m2);
}

vector_t centre_of_mass(body_t *b1, body_t *b2) {
    double m1 = body_get_mass(b1);
    double m2 = body_get_mass(b2);
    return weighted_vector_mean(body_get_centroid(b1),
                                m1,
                                body_get_centroid(b2),
                                m2);
}

void vec_fprint(FILE *stream, vector_t v) {
    fprintf(stream, "(%f, %f)", v.x, v.y);
}

/** TESTS **/

/**
 * Test (under)damped oscilation.
 * - Assert that mass follows correctly dampened trajectory.
 */
void test_underdamping(void) {
    const double M = 10;
    const double K = 2;
    const double A = 3;
    const double GAMMA = 2 * sqrt(M * K) / 100;
    const double DT = 1e-3;
    const int STEPS = 1000000;
    assert(GAMMA < 2 * sqrt(M * K) && "Is underdamped.");
    assert(A > 0);
    const double omega_damped = sqrt(4 * M * K - GAMMA * GAMMA) / M / 2;

    scene_t *scene = scene_init();
    body_t *body = make_body(M);
    body_set_centroid(body, (vector_t){A, 0});
    scene_add_body(scene, body);
    body_t *anchor = make_anchor();
    scene_add_body(scene, anchor);
    create_spring(scene, K, body, anchor);
    create_drag(scene, GAMMA, body);
    if (VERBOSE_TESTS) {
        fprintf(stderr, "underdamping,time,ref,engine\n");
    }
    for (int i = 0; i < STEPS; i++) {
        double x_ref
            = A * exp(-GAMMA * i * DT / M / 2) * cos(omega_damped * i * DT);
        if (VERBOSE_TESTS) {
            double x_engine = body_get_centroid(body).x;
            fprintf(stderr, "underdamping,%f,%f,%f\n", i * DT, x_ref, x_engine);
        }
        assert(
            DONT_FAIL
            || vec_within(5e-2, body_get_centroid(body), (vector_t){x_ref, 0}));
        assert(DONT_FAIL || vec_equal(body_get_centroid(anchor), VEC_ZERO));
        scene_tick(scene, DT);
    }
    scene_free(scene);
}

/**
 * Test whether terminal velocity is reached and not exceeded while falling in
 * gravitational field with linear drag.
 *
 * See http://hyperphysics.phy-astr.gsu.edu/hbase/lindrg2.html.
 */
void test_terminal_velocity(void) {
    const long double M_BODY = 10;
    const long double M_PLANET = 1e7;
    const long double R_PLANET = 1e3;
    const long double R_HEIGHT = 1; // Height off ground body starts.
    const long double G = 1;
    const long double G_ACCEL = G * M_PLANET / R_PLANET / R_PLANET;
    const long double TAU = 1e-2; // Characteristic time.
    const long double GAMMA = M_BODY / TAU;
    const long double SPEED_TERMINAL = -M_BODY * G_ACCEL / GAMMA;
    // (How close to terminal velocity we consider to be asymptotic.)
    const long double EPSILON = 1e-2;
    const double DT = 1e-6;
    const int STEPS = 1000000;
    // (Steps after which we require asymptotic behavior.)
    const int STEPS_TILL_ASYMP = STEPS * 9 / 10;
    assert(STEPS_TILL_ASYMP < STEPS); // Avoid precision errors just in case.

    scene_t *scene = scene_init();
    body_t *planet = make_body(M_PLANET);
    body_t *body = make_body(M_BODY);
    body_set_centroid(body, (vector_t){R_PLANET + R_HEIGHT, 0});
    scene_add_body(scene, planet);
    scene_add_body(scene, body);
    create_newtonian_gravity(scene, G, planet, body);
    create_drag(scene, GAMMA, body);

    if (VERBOSE_TESTS) {
        fprintf(stderr, "G_ACCEL: %Lf\n", G_ACCEL);
        fprintf(stderr, "time,x,speed_terminal,speed_engine\n");
    }

    int i;
    // Get to within epsilon of terminal velocity.
    for (i = 0; i < STEPS_TILL_ASYMP; i++) {
        double x_engine = body_get_centroid(body).x;
        double vx_engine = body_get_velocity(body).x;
        if (VERBOSE_TESTS) {
            fprintf(stderr,
                    "%f,%f,%Lf,%f\n",
                    i * DT,
                    x_engine,
                    SPEED_TERMINAL,
                    vx_engine);
        }
        if (within(EPSILON, SPEED_TERMINAL, vx_engine)) {
            break;
        }
        scene_tick(scene, DT);
    }
    // Assert stay within epsilon of terminal velocity.
    for (; i < STEPS; i++) {
        double x_engine = body_get_centroid(body).x;
        double vx_engine = body_get_velocity(body).x;
        if (VERBOSE_TESTS) {
            fprintf(stderr,
                    "%f,%f,%Lf,%f\n",
                    i * DT,
                    x_engine,
                    SPEED_TERMINAL,
                    vx_engine);
        }
        assert(DONT_FAIL || within(EPSILON, SPEED_TERMINAL, vx_engine));
        scene_tick(scene, DT);
    }

    scene_free(scene);
}

/**
 * Test that the centre of mass of two bodies with gravitational attraction
 * moves linearly as expected.
 */
void test_two_body_cm(void) {
    const double M1 = 4.5, M2 = 7.3;
    const double G = 1e3;
    const vector_t D_INITIAL = {5, 15}; // Initial difference vector bt M1, M2.
    const vector_t CM_INITAL
        = weighted_vector_mean(VEC_ZERO, M1, D_INITIAL, M2);
    const vector_t V2_INITIAL = {5, -1}; // Initial velocity of M2.
    const vector_t V_CM_INITIAL
        = weighted_vector_mean(VEC_ZERO, M1, V2_INITIAL, M2);
    const double DT = 1e-6;
    const int STEPS = 1000000;

    scene_t *scene = scene_init();
    body_t *body1 = make_body(M1);
    body_t *body2 = make_body(M2);
    body_set_centroid(body2, D_INITIAL);
    body_set_velocity(body2, V2_INITIAL);
    scene_add_body(scene, body1);
    scene_add_body(scene, body2);
    create_newtonian_gravity(scene, G, body1, body2);

    if (VERBOSE_TESTS) {
        fprintf(stderr, "time,cm_ref,cm_engine\n");
    }

    for (int i = 0; i < STEPS; i++) {
        vector_t cm_ref
            = vec_add(CM_INITAL, vec_multiply(i * DT, V_CM_INITIAL));
        vector_t cm_engine = centre_of_mass(body1, body2);
        if (VERBOSE_TESTS) {
            fprintf(stderr, "%f,", i * DT);
            vec_fprint(stderr, cm_ref);
            fprintf(stderr, ",");
            vec_fprint(stderr, cm_engine);
            fprintf(stderr, "\n");
        }
        assert(DONT_FAIL
               || (vec_isclose(cm_ref, centre_of_mass(body1, body2))
                   && vec_isclose(velocity_cm(body1, body2), V_CM_INITIAL)));
        scene_tick(scene, DT);
    }

    scene_free(scene);
}

/** MAIN **/

int main(int argc, char *argv[]) {
    // Run all tests if there are no command-line arguments
    bool all_tests = argc == 1;
    // Read test name from file
    char testname[100];
    if (!all_tests) {
        read_testname(argv[1], testname, sizeof(testname));
    }

    DO_TEST(test_underdamping);
    DO_TEST(test_terminal_velocity);
    DO_TEST(test_two_body_cm);

    puts("student_tests PASS");
}
