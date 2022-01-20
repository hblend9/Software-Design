#include "movement.h"
#include "body.h"
#include "collision.h"
#include "forces.h"
#include "list.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void hippo_movement_side(body_t *hippo,
                         vector_t center_world,
                         vector_t initial_position,
                         double radius,
                         double angle,
                         double speed,
                         double direction) {
    assert(fabs(direction) == 1.0); // Why is this a double????
    double small_angle = direction * speed;
    vector_t center_hippo = body_get_centroid(hippo);
    vector_t proposed_new_centroid
        = vec_rotate_relative(center_hippo, small_angle, center_world);
    // first check if you're under the max angle, in which case either direction
    // is okay to move in second check if you're over the max angle, but heading
    // back (i.e decreasing abs value of the angle from the initial position
    // compared to where you are now), its okay to move
    if ((fabs(vec_angle_between(initial_position, center_hippo, center_world))
         < fabs(angle * direction))
        || (fabs(vec_angle_between(initial_position,
                                   proposed_new_centroid,
                                   center_world))
            < fabs(vec_angle_between(initial_position,
                                     center_hippo,
                                     center_world)))) {
        body_set_centroid(hippo, proposed_new_centroid);
        body_rotate(hippo, small_angle);
    }
}

void hippo_movement_forward(body_t *hippo,
                            vector_t center_world,
                            double rescale_factor) {
    vector_t center_hippo = body_get_centroid(hippo);
    vector_t difference = vec_subtract(center_world, center_hippo);
    // trying to prevent hippo heads from all colliding in the center
    vector_t vec_move_by = vec_multiply(rescale_factor, difference);
    body_translate(hippo, vec_move_by);
}

void hippo_movement_backward(body_t *hippo,
                             vector_t center_world,
                             double forward_rescale_factor,
                             double radius) {
    vector_t center_hippo = body_get_centroid(hippo);
    vector_t difference = vec_subtract(center_world, center_hippo);
    // reverse the changes made by hippo_movement_forward
    double rescale_factor = radius - vec_magnitude(difference);
    vector_t norm_difference
        = vec_multiply((1.0 / vec_magnitude(difference)), difference);
    vector_t vec_move_by = vec_multiply(-rescale_factor, norm_difference);
    body_translate(hippo, vec_move_by);
}
