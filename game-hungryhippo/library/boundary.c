#include "boundary.h"
#include "body.h"
#include "forces.h"
#include "list.h"
#include "movement.h"
#include "vector.h"
#include <assert.h>
#include <collision.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const double OFFSET = 10;
const double BUFFER_CENTER = 3;

bool outside_boundary(body_t *body,
                      vector_t center_world,
                      double radius,
                      double buffer_zone) {
    vector_t center_body = body_get_centroid(body);
    double distance = vec_magnitude(vec_subtract(center_body, center_world));
    return (distance > radius + buffer_zone);
}

void reset_boundary_balls(body_t *ball,
                          vector_t center_world,
                          double radius,
                          double buffer_zone,
                          double initial_velocity) {
    if (outside_boundary(ball, center_world, radius, buffer_zone)) {
        // move the ball back to the center of the world
        body_set_centroid(ball, center_world);
        // give it its initial velocity again * the direction it is in currently
        vector_t curr_vel = body_get_velocity(ball);
        body_set_velocity(
            ball,
            vec_multiply((initial_velocity / vec_magnitude(curr_vel)),
                         curr_vel));
    }
}

void boundary_arena_ball_collision(body_t *ball,
                                   vector_t center_world,
                                   double radius,
                                   double buffer_radius,
                                   double buffer_zone,
                                   double initial_vel_factor) {
    // only does things if the ball is in the sweet spot between being outside
    // the radius but within the radius + the buffer zone
    if ((outside_boundary(ball, center_world, radius, 0))
        && (!outside_boundary(ball, center_world, radius, buffer_zone))) {

        // calculate the point on the circle closest to the ball currently based
        // on the angle its at eq circle: radius^2 = x-center_world.x^2 +
        // y-center_world.y^2
        vector_t bottom_ref_point
            = vec_subtract(center_world,
                           vec_multiply((radius - buffer_radius), E2));
        vector_t old_center = body_get_centroid(ball);
        double angle_phi
            = vec_angle_between(old_center, bottom_ref_point, center_world);

        // move the ball to the closest point on the circle
        vector_t new_centroid
            = vec_rotate_relative(bottom_ref_point, -angle_phi, center_world);
        body_set_centroid(ball, new_centroid);

        // calculate tan line at that point
        vector_t recenter = vec_subtract(new_centroid, center_world);
        double slope_perpendicular = recenter.y / recenter.x;
        double slope_tan = -1.0 / slope_perpendicular;

        // find angle between ball's curr velocity and tan line using an
        // intermediary point
        vector_t on_tan_offset = {.x = new_centroid.x + OFFSET,
                                  .y = (new_centroid.y + slope_tan * OFFSET)};
        vector_t old_vel = body_get_velocity(ball);
        double angle_theta = vec_angle_between(on_tan_offset,
                                               vec_add(old_vel, new_centroid),
                                               new_centroid);

        // rotate the curr velocity by 2 * angle_theta over the tan line (flip
        // it, incident angle)
        vector_t new_vel = vec_rotate(old_vel, -2 * angle_theta);
        vector_t new_vel_normalized
            = vec_multiply((initial_vel_factor / vec_magnitude(new_vel)),
                           new_vel);

        // set the ball's new velocity
        body_set_velocity(ball, new_vel_normalized);
    }
}
