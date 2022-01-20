#include "key_listener.h"
#include "list.h"
#include <assert.h>

/*** STRUCTURES ***/
struct key_listener {
    // (In the current implementation, a key is repeated in the keys list as
    // many times as it has an ear listening for it; one way to improve this is
    // to make ear_trackers a list of lists, each list cooresponding to a
    // particular key.)
    list_t *keys;         // 'SDL_Keycode's that we're listening for.
    list_t *ear_trackers; // List of ears listening to corresponding keys.
};

/**
 * A simple struct that keeps track of an ear and its aux.
 */
typedef struct _ear_tracker {
    ear_func_t ear;
    void *aux;
    free_func_t freer;
} _ear_tracker_t;

/*** PRIVATE PROTOTYPES ***/
void _ear_tracker_free(_ear_tracker_t *ear_tracker);

/*** DEFINITIONS ***/

key_listener_t *key_listener_init(void) {
    key_listener_t *key_listener = malloc(sizeof(key_listener_t));
    key_listener->keys = list_init(1, free);
    key_listener->ear_trackers = list_init(1, (free_func_t)_ear_tracker_free);
    return key_listener;
}

void _ear_tracker_free(_ear_tracker_t *ear_tracker) {
    if (ear_tracker->freer != NULL) {
        ear_tracker->freer(ear_tracker->aux);
    }
    free(ear_tracker);
}

void key_listener_free(key_listener_t *key_listener) {
    list_free(key_listener->keys);
    list_free(key_listener->ear_trackers);
    free(key_listener);
}

void key_listener_refresh(key_listener_t *key_listener) {
    list_free(key_listener->keys);
    list_free(key_listener->ear_trackers);
    key_listener->keys = list_init(1, free);
    key_listener->ear_trackers = list_init(1, (free_func_t)_ear_tracker_free);
}

void key_listener_add(key_listener_t *key_listener,
                      ear_func_t ear,
                      void *aux,
                      SDL_Keycode key,
                      free_func_t freer) {
    SDL_Keycode *key_p = malloc(sizeof(SDL_Keycode));
    assert(key_p != NULL);
    *key_p = key;
    _ear_tracker_t *ear_tracker = malloc(sizeof(_ear_tracker_t));
    assert(ear_tracker != NULL);
    ear_tracker->ear = ear;
    ear_tracker->aux = aux;
    ear_tracker->freer = freer;
    list_add(key_listener->keys, key_p);
    list_add(key_listener->ear_trackers, ear_tracker);
    assert(list_size(key_listener->keys)
           == list_size(key_listener->ear_trackers));
}

void key_listener_listen(key_listener_t *key_listener,
                         SDL_KeyboardEvent event) {
    SDL_Keycode key = event.keysym.sym;
    for (size_t i = 0; i < list_size(key_listener->keys); i++) {
        if (*(SDL_Keycode *)list_get(key_listener->keys, i) == key) {
            _ear_tracker_t *ear_tracker
                = list_get(key_listener->ear_trackers, i);
            ear_tracker->ear(event, ear_tracker->aux);
            break;
        }
    }
}
