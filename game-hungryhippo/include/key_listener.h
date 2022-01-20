#ifndef __KEY_LISTENER_H__
#define __KEY_LISTENER_H__

#include <SDL2/SDL.h>

typedef void (*free_func_t)(void *);

/**
 * A key listener which can be passed all sdl key events and delegate as needed.
 * The things that actually "listen" for events are called "ears" and can be
 * added below.
 */
typedef struct key_listener key_listener_t;

/**
 * A function that is called when a certain key is pressed. The event is passed
 * by value because SDL seems to follow the convention that events have very
 * short lifecycles, and even if they are passed reference they are copied on
 * the other end. To make this practice explict, we forbid passing events by
 * reference.
 */
typedef void (*ear_func_t)(SDL_KeyboardEvent event, void *aux);

/**
 * Create a new key listener.
 * The client is responsible for setting up SDL and calling key_listener_listen
 * inside of the main event loop. In particular, if you don't want repeat keys,
 * you should filter for those in the main event loop.
 */
key_listener_t *key_listener_init(void);

/**
 * Free key listener and any auxs whose freers were not NULL.
 */
void key_listener_free(key_listener_t *key_listener);

/**
 * Listen to a key event and delegate to the appropriate ears.
 * This should be placed in your main event loop and only passed an
 * SDL_KeyboardEvent.
 */
void key_listener_listen(key_listener_t *key_listener, SDL_KeyboardEvent event);

/**
 * Register an ear with one or more keys.
 * When one of the specified key appears in an event, the event and the aux are
 * passed to the ear function.
 */
void key_listener_add(key_listener_t *key_listener,
                      ear_func_t ear,
                      void *aux,
                      SDL_Keycode key,
                      free_func_t freer);

/**
 * Remove all ears from the key listener and free any auxs whose freers are
 * non-NULL. This takes effect immediately, i.e. is not deferred.
 */
void key_listener_refresh(key_listener_t *key_listener);

#endif // #ifnded __KEY_LISTENER_H__
