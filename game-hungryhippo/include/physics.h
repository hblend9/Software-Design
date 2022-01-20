#ifndef __PHYSICS_H__
#define __PHYSICS_H__

/*** DEPENDENCY FORWARD DECLARATIONS ***/
typedef struct list list_t;
typedef void (*free_func_t)(void *);

/*** INTERFACE ***/

/**
 * A physics engine wrapper that keeps track of bodies and forces therebetween.
 */
typedef struct physics physics_t;

/**
 * A function which adds some forces or impulses to bodies,
 * e.g. from collisions, gravity, or spring forces.
 * Takes in an auxiliary value that can store parameters or state.
 */
typedef void (*force_creator_t)(void *aux);

/**
 * Create a new physics layer.
 */
physics_t *physics_init(void);

/**
 * Free the physics layer and associates auxilliaries but not its bodies.
 */
void physics_free(physics_t *physics);

/**
 * Add a group of bodies to the physics layer, which will cause them to be
 * ticked each tick and effected by any forces associated with them within this
 * layer. Forces can be created between any two bodies in the entire physics
 * layer, not just their own group. Bodies that are marked for removal via
 * 'body_remove' are removed from the physics layer automatically, but not freed
 * here (hint: game actually frees things!).
 */
void physics_add_bodies(physics_t *physics, list_t *bodies);

/**
 * Add a force creator to the layer, to be invoked each tick. Can involve bodies
 * from any of the groups that have been added to the physics layer.
 */
void physics_add_force(physics_t *physics,
                       force_creator_t forcer,
                       void *forcer_aux,
                       list_t *bodies,
                       free_func_t aux_freer);

/**
 * Tick the physics layer forward dt seconds, i.e. tick bodies forward with any
 * associated forces. Also remove bodies marked for removal but do not free
 * them.
 */
void physics_tick(physics_t *physics, double dt);

#endif // #ifndef __PHYSICS_H__
