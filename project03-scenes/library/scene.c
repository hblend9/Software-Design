#ifndef __SCENE_H__
#define __SCENE_H__

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include "body.h"
#include "list.h"

const size_t INIT_CAPACITY = 100;

/**
 * A collection of bodies.
 * The scene automatically resizes to store arbitrarily many bodies.
 */
typedef struct scene{
    list_t *bodies;
    size_t size;
}scene_t;

/**
 * Allocates memory for an empty scene.
 * Makes a reasonable guess of the number of bodies to allocate space for.
 * Asserts that the required memory is successfully allocated.
 *
 * @return the new scene
 */
scene_t *scene_init(void)
{
    scene_t *scene = malloc(sizeof(scene_t));
    assert(scene != NULL);
    scene->bodies = list_init(INIT_CAPACITY, (free_func_t) body_free);
    scene->size = 0;
    return scene;
}

/**
 * Releases memory allocated for a given scene and all its bodies.
 *
 * @param scene a pointer to a scene returned from scene_init()
 */
void scene_free(scene_t *scene){
    list_free(scene->bodies);
    free(scene);   
}

/**
 * Gets the number of bodies in a given scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @return the number of bodies added with scene_add_body()
 */
size_t scene_bodies(scene_t *scene){
    return list_size(scene->bodies);
}

/**
 * Gets the body at a given index in a scene.
 * Asserts that the index is valid.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param index the index of the body in the scene (starting at 0)
 * @return a pointer to the body at the given index
 */
body_t *scene_get_body(scene_t *scene, size_t index){
    assert(index < scene_bodies(scene));
    return list_get(scene->bodies, index);
}

/**
 * Adds a body to a scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param body a pointer to the body to add to the scene
 */
void scene_add_body(scene_t *scene, body_t *body){
    list_add(scene->bodies, body);
    scene->size++;
}

/**
 * Removes and frees the body at a given index from a scene.
 * Asserts that the index is valid.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param index the index of the body in the scene (starting at 0)
 */
void scene_remove_body(scene_t *scene, size_t index){
    body_t *removed = list_remove(scene->bodies, index);
    body_free(removed);
    scene->size--;
}

/**
 * Executes a tick of a given scene over a small time interval.
 * This requires ticking each body in the scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param dt the time elapsed since the last tick, in seconds
 */
void scene_tick(scene_t *scene, double dt){
    for (size_t i = 0; i < list_size(scene->bodies); i++){
        body_tick(list_get(scene->bodies, i), dt);
    }
}

#endif // #ifndef __SCENE_H__
