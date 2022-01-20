#ifndef __BODY_H__
#define __BODY_H__

#include "color.h"
#include "list.h"
#include "vector.h"
#include <stdbool.h>

/* Structs that this module depends on. */
typedef struct gfx_aux gfx_aux_t;

/**
 * A rigid body constrained to the plane.
 * Implemented as a polygon with uniform density.
 * Bodies can accumulate forces and impulses during each tick.
 * Angular physics (i.e. torques) are not currently implemented.
 * Contains a graphical aux (gfx_aux_t), but does not know anything else about
 * graphics; use gfx_aux.h for graphics.
 */
typedef struct body body_t;

/**
 * Initializes a body without any info.
 * Acts like body_init_with_info() where info and info_freer are NULL.
 */
body_t *body_init(list_t *shape, double mass, rgb_color_t color);

/**
 * Allocates memory for a body with the given parameters.
 * The body is initially at rest.
 * Asserts that the mass is positive and that the required memory is allocated.
 *
 * @param shape a list of vectors describing the initial shape of the body
 * @param mass the mass of the body (if INFINITY, stops the body from moving)
 * @param color the color of the body, used to draw it on the screen
 * @param info additional information to associate with the body,
 *   e.g. its type if the scene has multiple types of bodies
 * @param info_freer if non-NULL, a function call on the info to free it
 * @return a pointer to the newly allocated body
 */
body_t *body_init_with_info(list_t *shape,
                            double mass,
                            rgb_color_t color,
                            void *info,
                            free_func_t info_freer);

/**
 * Initialize a body with more advancedTM graphical properties and multiple
 * shapes. The active shape defaults to the zeroth one.
 */
body_t *body_init_with_gfx(double mass,
                           void *info,
                           free_func_t info_freer,
                           list_t *shapes,
                           gfx_aux_t *gfx_aux);

/**
 * Releases the memory allocated for a body.
 *
 * @param body a pointer to a body returned from body_init()
 */
void body_free(body_t *body);

/**
 * Return the currently active shape of a body.
 * *Does* return a copy.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the polygon describing the body's current position
 */
list_t *body_get_shape(body_t *body);

/**
 * Return a pointer that will always be updated to point to the body's main
 * shape.
 */
list_t **body_get_shape_main_p(body_t *body);

/**
 * Return the currently active shape of a body.
 * Does *NOT* return a copy.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the polygon describing the body's current position
 */
list_t *body_get_shape_nocp(body_t *body);

/**
 * Return a pointer to the body's main shape.
 * The pointer will always be updated to point to the body' main shape in the
 * future, so use this if you want to persistently access the body's main shape
 * in the future.
 */
list_t *body_get_shape_main(body_t *body);

/**
 * Return the body's idx'th shape (NOT a copy).
 */
list_t *body_get_shape_alt(body_t *body, size_t idx);

/**
 * Return the number of shapes tracked by the body.
 */
size_t body_get_num_shapes(body_t *body);

/**
 * Get a shape of the body addressed by index.
 */

/**
 * Gets the current center of mass of a body.
 * While this could be calculated with polygon_centroid(), that becomes too slow
 * when this function is called thousands of times every tick.
 * Instead, the body should store its current centroid.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the body's center of mass
 */
vector_t body_get_centroid(body_t *body);

/**
 * Return a pointer to the body's centroid, as would be returned by
 * body_get_centroid.
 */
vector_t *body_get_anchor(body_t *body);

/**
 * Gets the current velocity of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the body's velocity vector
 */
vector_t body_get_velocity(body_t *body);

/**
 * Gets the mass of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the mass passed to body_init(), which must be greater than 0
 */
double body_get_mass(body_t *body);

/**
 * Sets the inertia of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @param inertia the inertia of the body
 */
void body_set_inertia(body_t *body, double inertia);

/**
 * Gets the inertia of a body.
 *
 * @param body a pointer to a body returned from body_init()
 */
double body_get_inertia(body_t *body);

/**
 * Gets the display color of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the color passed to body_init(), as an (R, G, B) tuple
 */
rgb_color_t body_get_color(body_t *body);

/**
 * Gets the information associated with a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the info passed to body_init()
 */
void *body_get_info(body_t *body);

/**
 * Reset the info of the body.
 */
void body_set_info(body_t *body, void *new_info, free_func_t freer);

/**
 * Translates a body to a new position.
 * The position is specified by the position of the body's center of mass.
 *
 * @param body a pointer to a body returned from body_init()
 * @param x the body's new centroid
 */
void body_set_centroid(body_t *body, vector_t x);

/**
 * Changes a body's velocity (the time-derivative of its position).
 *
 * @param body a pointer to a body returned from body_init()
 * @param v the body's new velocity
 */
void body_set_velocity(body_t *body, vector_t v);

/**
 * Changes a body's orientation in the plane.
 * The body is rotated about its center of mass.
 * Note that the angle is *absolute*, not relative to the current orientation.
 *
 * @param body a pointer to a body returned from body_init()
 * @param angle the body's new angle in radians. Positive is counterclockwise.
 */
void body_set_rotation(body_t *body, double angle);

/**
 * Applies a force to a body over the current tick.
 * If multiple forces are applied in the same tick, they should be added.
 * Should not change the body's position or velocity; see body_tick().
 *
 * @param body a pointer to a body returned from body_init()
 * @param force the force vector to apply
 */
void body_add_force(body_t *body, vector_t force);

/**
 * Applies an impulse to a body.
 * An impulse causes an instantaneous change in velocity,
 * which is useful for modeling collisions.
 * If multiple impulses are applied in the same tick, they should be added.
 * Should not change the body's position or velocity; see body_tick().
 *
 * @param body a pointer to a body returned from body_init()
 * @param impulse the impulse vector to apply
 */
void body_add_impulse(body_t *body, vector_t impulse);

/**
 * Updates the body after a given time interval has elapsed.
 * Sets acceleration and velocity according to the forces and impulses
 * applied to the body during the tick.
 * The body should be translated at the *average* of the velocities before
 * and after the tick.
 * Resets the forces and impulses accumulated on the body.
 *
 * @param body the body to tick
 * @param dt the number of seconds elapsed since the last tick
 */
void body_tick(body_t *body, double dt);

/**
 * Marks a body for removal--future calls to body_is_removed() will return true.
 * Does not free the body.
 * If the body is already marked for removal, does nothing.
 *
 * @param body the body to mark for removal
 */
void body_remove(body_t *body);

/**
 * Returns whether a body has been marked for removal.
 * This function returns false until body_remove() is called on the body,
 * and returns true afterwards.
 *
 * @param body the body to check
 * @return whether body_remove() has been called on the body
 */
bool body_is_removed(body_t *body);

/**
 * Deprecated.
 * Performs a deep copy of the memory allocated in the original body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return a copy of the body
 */
body_t *body_copy(body_t *body);

/**
 * Set the body's colour to a rgb_color_t.
 */
void body_set_color(body_t *body, rgb_color_t value);

/**
 * Translate a body relative to its current location.
 *
 * @param body
 * @param translation The translation vector which will be added to the body's
 * current location.
 */
void body_translate(body_t *body, vector_t translation);

/**
 * Rotate a body relative to its current orientation.
 *
 * @param body
 * @param angle Relative angle by which to rotate. Positive is anticlockwise.
 */
void body_rotate(body_t *body, double angle);

/**
 * Return the body's angle, relative to initial orientation.
 */
double body_get_rotation(body_t *body);

/**
 * Return pointer to the body's angle, as would be returned by
 * body_get_rotation.
 */
double *body_get_angle_p(body_t *body);

/**
 * Return the body's angular velocity.
 */
double body_get_angular_velocity(body_t *body);

/**
 * Return the body's angular acceleration.
 */
double body_get_angular_acceleration(body_t *body);

/**
 * Set the body's angular velocity to a new signed double value. Positive is
 * anticlockwise.
 */
void body_set_angular_velocity(body_t *body, double value);

/**
 * Set the body's angular acceleration to a new signed double value. Positive is
 * anticlockwise.
 */
void body_set_angular_acceleration(body_t *body, double value);

/**
 * Set all angular dynamics of the body to second-order. Our model approximates
 * that angular acceleration is constant over each tick. In all of the below
 * quantities, positive means anticlockwise.
 *
 * @param body
 * @param angle The body's new absolute rotation (see 'body_set_rotation').
 * @param velocity The body's new angular velocity as a signed double.
 * @param acceleration The body's new angular acceleration as a signed double.
 */
void body_set_angular_dynamics(body_t *body,
                               double angle,
                               double angular_velocity,
                               double angular_acceleration);

/**
 * Return the body's acceleration.
 * @deprecated, use forces instead.
 *
 * @param body
 * @return acceleration as a vector.
 */
vector_t body_get_acceleration(body_t *body);

/**
 * Set the body's acceleration to a new vector value.
 * @deprecated, use forces instead.
 */
void body_set_acceleration(body_t *body, vector_t value);

/**
 * Set the body's main shape, addressed by index.
 */
void body_set_shape_main(body_t *body, size_t idx);

/**
 * Get a pointer to the body's gfx_aux.
 * Return NULL if the body doesn't have gfx. In that case, graphics are handled
 * by the body's main shape.
 */
gfx_aux_t *body_get_gfx(body_t *body);

#endif // #ifndef __BODY_H__
