#ifndef __FORCES_H__
#define __FORCES_H__

#include <stdbool.h>
#include <stdlib.h>

/*** DEPENDENCY FORWARD DECLARATIONS ***/
typedef struct body body_t;
typedef struct list list_t;
typedef struct physics physics_t;
typedef struct vector vector_t;
typedef void (*free_func_t)(void *);
typedef void (*force_creator_t)(void *aux);

/**
 * A function called when a collision occurs.
 * @param body1 the first body passed to create_collision()
 * @param body2 the second body passed to create_collision()
 * @param axis a unit vector pointing from body1 towards body2
 *   that defines the direction the two bodies are colliding in
 * @param aux the auxiliary value passed to create_collision()
 */
typedef void (*collision_handler_t)(body_t *body1,
                                    body_t *body2,
                                    vector_t axis,
                                    vector_t collision_point,
                                    void *aux);

/**
 * Struct to keep track of parameters to be given to the force creator
 * functions.
 */
typedef enum aux_type { GENERAL_AUX, COLLISION_AUX } aux_type_e;

typedef struct aux {
    aux_type_e type;
    list_t *bodies;
    list_t *constants;
} aux_t;

typedef struct aux_collision {
    aux_type_e type;
    list_t *bodies;

    list_t **shape1_p;
    list_t **shape2_p;
    bool made_shape1_p;
    bool made_shape2_p;

    collision_handler_t handler;
    void *handler_aux;
    free_func_t handler_aux_freer;
    bool already_collided;
} aux_collision_t;

// Aux helper functions.
aux_t *aux_init(size_t n_consts, size_t n_bodies);
aux_collision_t *aux_collision_init(size_t n_bodies,
                                    collision_handler_t handler,
                                    void *handler_aux,
                                    free_func_t handler_aux_freer);
void aux_free(aux_t *aux);
void aux_add_body(aux_t *aux, body_t *body);
void aux_add_constant(aux_t *aux, double c);

/**
 * Adds a force creator to a physics that applies gravity between two bodies.
 * The force creator will be called each tick
 * to compute the Newtonian gravitational force between the bodies.
 * See
 * https://en.wikipedia.org/wiki/Newton%27s_law_of_universal_gravitation#Vector_form.
 * The force should not be applied when the bodies are very close,
 * because its magnitude blows up as the distance between the bodies goes to 0.
 *
 * @param physics the physics containing the bodies
 * @param G the gravitational proportionality constant
 * @param body1 the first body
 * @param body2 the second body
 */
void create_newtonian_gravity(physics_t *physics,
                              double G,
                              body_t *body1,
                              body_t *body2);

/**
 * Adds a force creator to a physics that acts like a spring between two bodies.
 * The force creator will be called each tick
 * to compute the Hooke's-Law spring force between the bodies.
 * See https://en.wikipedia.org/wiki/Hooke%27s_law.
 *
 * @param physics the physics containing the bodies
 * @param k the Hooke's constant for the spring
 * @param body1 the first body
 * @param body2 the second body
 */
void create_spring(physics_t *physics, double k, body_t *body1, body_t *body2);

/**
 * Adds a force creator to a physics that applies a drag force on a body.
 * The force creator will be called each tick
 * to compute the drag force on the body proportional to its velocity.
 * The force points opposite the body's velocity.
 *
 * @param physics the physics containing the bodies
 * @param gamma the proportionality constant between force and velocity
 *   (higher gamma means more drag)
 * @param body the body to slow down
 */
void create_drag(physics_t *physics, double gamma, body_t *body);

/**
 * Adds a force creator to a physics that calls a given collision handler
 * function each time two bodies collide.
 * This generalizes create_destructive_collision() from last week,
 * allowing different things to happen on a collision.
 * The handler is passed the bodies, the collision axis, and an auxiliary value.
 * It should only be called once while the bodies are still colliding.
 *
 * @param physics the physics containing the bodies
 * @param body1 the first body
 * @param body2 the second body
 * @param handler a function to call whenever the bodies collide
 * @param aux an auxiliary value to pass to the handler
 * @param freer if non-NULL, a function to call in order to free aux
 */
void create_collision(physics_t *physics,
                      body_t *body1,
                      body_t *body2,
                      collision_handler_t handler,
                      void *aux,
                      free_func_t freer);

/**
 * Just like create_collision but the shapes that are used to check for
 * collision are pointed to by shape{1,2}_p.
 */
void create_collision_shapes(physics_t *physics,
                             body_t *body1,
                             body_t *body2,
                             list_t **shape1_p,
                             list_t **shape2_p,
                             collision_handler_t handler,
                             void *aux,
                             free_func_t freer);

void create_collision_general(physics_t *physics,
                              body_t *body1,
                              body_t *body2,
                              list_t **shape1_p,
                              list_t **shape2_p,
                              collision_handler_t handler,
                              void *aux,
                              free_func_t freer,
                              force_creator_t force_creator);

/**
 * Adds a force creator to a physics that destroys two bodies when they collide.
 * The bodies should be destroyed by calling body_remove().
 * This should be represented as an on-collision callback
 * registered with create_collision().
 *
 * @param physics the physics containing the bodies
 * @param body1 the first body
 * @param body2 the second body
 */
void create_destructive_collision(physics_t *physics,
                                  body_t *body1,
                                  body_t *body2);

void create_hippo_ball_collision(physics_t *physics,
                                 body_t *ball,
                                 body_t *hippo);

/**
 * Adds a force creator to a physics that destroys only the first body when they
 * collide. The body should be destroyed by calling body_remove(). This should
 * be represented as an on-collision callback registered with
 * create_collision().
 *
 * @param physics the physics containing the bodies
 * @param body_to_remove the body to be removed
 * @param body_other the body that collides with it but should not be removed
 */
void create_asymmetric_destructive_collision(physics_t *physics,
                                             body_t *body_to_remove,
                                             body_t *body_other);

/**
 * Adds a force creator to a physics that applies impulses
 * to resolve collisions between two bodies in the physics.
 * This should be represented as an on-collision callback
 * registered with create_collision().
 *
 * You may remember from project01 that you should avoid applying impulses
 * multiple times while the bodies are still colliding.
 * You should also have a special case that allows either body1 or body2
 * to have mass INFINITY, as this is useful for simulating walls.
 *
 * @param physics the physics containing the bodies
 * @param elasticity the "coefficient of restitution" of the collision;
 * 0 is a perfectly inelastic collision and 1 is a perfectly elastic collision
 * @param body1 the first body
 * @param body2 the second body
 */
void create_physics_collision(physics_t *physics,
                              double elasticity,
                              body_t *body1,
                              body_t *body2);

void create_hippo_collision(physics_t *physics,
                            double elasticity,
                            body_t *hippo1,
                            body_t *hippo2);

void create_physics_collision_shapes(physics_t *physics,
                                     double elasticity,
                                     body_t *body1,
                                     body_t *body2,
                                     list_t **shape1_p,
                                     list_t **shape2_p);

/**
 * Adds a force creator to a physics that applies impulses
 * to resolve collisions between two bodies in the physics.
 * This should be represented as an on-collision callback
 * registered with create_collision().
 *
 * You may remember from project01 that you should avoid applying impulses
 * multiple times while the bodies are still colliding.
 * You should also have a special case that allows either body1 or body2
 * to have mass INFINITY, as this is useful for simulating walls.
 *
 * @param physics the physics containing the bodies
 * @param elasticity the "coefficient of restitution" of the collision;
 * 0 is a perfectly inelastic collision and 1 is a perfectly elastic collision
 * @param body1 the first body
 * @param body2 the second body
 */
void create_physics_spin_collision(physics_t *physics,
                                   double elasticity,
                                   double friction,
                                   body_t *body1,
                                   body_t *body2);
#endif // #ifndef __FORCES_H__
