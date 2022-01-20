#ifndef __MOVEMENT_H__
#define __MOVEMENT_H__

#include "body.h"

/**
 * hippo_movement_side moves the hippo in a circle of given radius around the
 * center_world point. The hippo is prevented from moving it is beyond the max
 * angle given
 * Direction of +1 = counterclockwise, direction of -1 = clockwise
 * Note: does not rotate the hippo - it probably should or make hungryhippos
 * deal with this
 * @param hippo the body pointer to the current hippo
 * @param center_world the point at the center of the world
 * @param initial_position the fixed initial position of the hippo for reference
 *                          to determine if the hippo has gone too far
 * @param radius the radius of the world
 * @param angle the maximum angle the hippo is allowed to move side to side
 * @param speed the speed at which the hippo should move
 * @param direction the clockwise or counterclockwise direction the hippo should
 * mvove
 */
void hippo_movement_side(body_t *hippo,
                         vector_t center_world,
                         vector_t initial_position,
                         double radius,
                         double angle,
                         double speed,
                         double direction);

/**
 * hippo_movement_forward instantaneously moves the hippo forward by 2/3 of the
 * difference between the hippo's center and the center of the circle (to
 * prevent hippos from colliding in the center of the world).
 * @param hippo the body pointer to the current hippo
 * @param center_world the point center of the world the hippos are in
 * @param rescale_factor the amount of the difference between the center of the
 * world and the current position of the hippo should be covered in one move
 * forward (recommended 2/3)
 */
void hippo_movement_forward(body_t *hippo,
                            vector_t center_world,
                            double rescale_factor);

/**
 * hippo_movement_backward instantaneously moves the hippo backward by 2/3 of
 * the original difference between the hippo's center and the center of the
 * circle (so 2 x the difference currently). Reverses effects of
 * hippo_movement_forward
 * @param hippo the body pointer to the current hippo
 * @param center_world the point center of the world the hippos are in
 * @param forward_rescale_factor the amount that the hippo moved forward by
 * @param radius the radius of the world
 */
void hippo_movement_backward(body_t *hippo,
                             vector_t center_world,
                             double forward_rescale_factor,
                             double radius);

#endif // #ifndef __MOVEMENT_H__
