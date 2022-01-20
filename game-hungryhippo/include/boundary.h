#ifndef __BOUNDARY_H__
#define __BOUNDARY_H__

#include "body.h"
#include <stdbool.h>

// General Idea:
// if the ball is outside the radius but inside the buffer, redirect it
// if the ball is wayyy outside the radius (> buffer outside), move it back to
// the center

/**
 * outside_boundary checks whether a body is outside the given radius + buffer
 * of the arena returns true if the body is outside the boundary and false
 * otherwise
 * @param body the body to check inside/outside boundary status of
 * @param center_world the vector center of the world
 * @param radius the radius of the world
 * @param buffer_zone the buffer the body is allowed to be outside the radius of
 * the world
 */
bool outside_boundary(body_t *body,
                      vector_t center_world,
                      double radius,
                      double buffer_zone);

/**
 * reset_boundary_balls is an extreme function that checks if a ball has yeeted
 * itself through the "buffer" zone and is unrecoverable in the world
 * Should the ball be extrememly far outside the boundary, this function puts it
 * at the center of the arena with a lower velocity in the same direction
 * @param ball the body to move back inside the world
 * @param center_world the vector center of the world
 * @param radius the radius of the world
 * @param buffer_zone the buffer boundary beyond which the body is unrecoverable
 * @param initial_velocity the magnitude of the velocity the ball should start
 * with
 */
void reset_boundary_balls(body_t *ball,
                          vector_t center_world,
                          double radius,
                          double buffer_zone,
                          double initial_velocity);

/**
 * boundary_arena_ball_collision is called when the ball is moving at a decent
 * speed
 * and is just barely over the edge of colliding with the arena (within
 * buffer_zone). \ The ball is then moved back to radius away and a new velocity
 * is calculated for it based on its incident angle with the tangent line to the
 * circle at that point
 * @param ball the body to bounce off the arena
 * @param center_world the vector center of the world
 * @param radius the radius of the world
 * @param buffer_radius for appearance purposes, helps figure out where the edge
 * of the body is relative to the centroid so that "edges" bounce
 * @param buffer_zone the buffer boundary beyond which the body is unrecoverable
 * @param initial_velocity the magnitude of the velocity the ball should start
 * with
 */
void boundary_arena_ball_collision(body_t *ball,
                                   vector_t center_world,
                                   double radius,
                                   double buffer_radius,
                                   double buffer_zone,
                                   double initial_vel_factor);

#endif // #ifndef __BOUNDARY_H__
