#ifndef __KEY_HANDLING_H__
#define __KEY_HANDLING_H__

#include "game_world.h"
#include "list.h"
#include "sdl_wrapper.h"

typedef enum KEY_ENUM_INDEX {
    // player 1
    KEY_LEFT,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN,
    KEY_SLASH,
    // player 2
    KEY_A,
    KEY_D,
    KEY_W,
    KEY_S,
    KEY_E,
    // player 3
    KEY_H,
    KEY_L,
    KEY_K,
    KEY_J,
    KEY_I,
    // player 4
    KEY_1,
    KEY_2,
    KEY_5,
    KEY_3,
    KEY_4,
    // ensure max size is mutable
    MAX_SIZE,
    // passthrough for unrecognized keys
    KEY_PASSTHROUGH
} key_index_e;

typedef void (*key_handler_enumed_t)(key_index_e key,
                                     key_event_type_t type,
                                     void *aux);

typedef struct hh_key_aux {
    game_world_t *game;
    player_t **player_array;
    bool *already_shot_this_press_array;
    bool *already_switched_this_press_array;
    bool *bool_keys;
    key_handler_enumed_t key_handler;
} hh_key_aux_t;

/**
 * hh_key_aux_init initializes the key_aux to be used when
 * hh_on_key is called.
 * @param game the game that stores the info for the key_aux
 * @param key_handler the function that will be called from sdl every time there
 * is a new key event
 */
hh_key_aux_t *hh_key_aux_init(game_world_t *game,
                              key_handler_enumed_t key_handler);

/**
 * hh_key_aux_reset resets the key_aux to its initial values
 * but for the updated hippos in game_world
 * @param game the game that stores the info for the key_aux
 * @param aux the old aux that will be updated
 */
hh_key_aux_t *hh_key_aux_reset(game_world_t *game, hh_key_aux_t *aux);

/**
 * kh_currently_shot returns true if the hippo has already shot a ball on this
 * press and false otherwise
 * @param index the index of the current hippo
 * @param aux the extra information storage unit this function needs
 */
bool kh_currently_shot(hh_key_aux_t *aux, size_t index);

/**
 * kh_currently_switched returns true if the hippo has already switched a
 * powerup on this press and false otherwise
 * @param index the index of the current hippo
 * @param aux the extra information storage unit this function needs
 */
bool kh_currently_switched(hh_key_aux_t *aux, size_t index);

/**
 * kh_currently_pressed returns true if the key is being pressed
 * and hasn't yet been released or false if they key isn't pressed
 * @param index the index of the current key
 * @param aux the extra information storage unit this function needs
 */
bool kh_currently_pressed(key_index_e index, hh_key_aux_t *aux);

/**
 * kh_currently_pressed returns true if the key is being pressed
 * and hasn't yet been released or false if they key isn't pressed
 * @param key the current key
 * @param aux the extra information storage unit this function needs
 */
bool kh_currently_pressed_key(char key, hh_key_aux_t *aux);

/**
 * kh_map_key_event maps keys to their enums
 * @param key the character associated with the key event
 */
key_index_e kh_map_key_event(char key);

/**
 * kh_update_bool_list_char updates the array of booleans
 * associated with each key based on the current key event
 * @param key the current key
 * @param type the type of key event
 * @param held_time the amount fo time the key has been held
 * @param aux the hh_aux that contains the list of bools
 */
void kh_update_bool_list_char(char key,
                              key_event_type_t type,
                              double held_time,
                              hh_key_aux_t *aux);

/**
 * kh_update_bool_list updates the array of booleans associated with each
 * key based on the current key event
 * @param index the index of the current key
 * @param type the type of key event
 * @param held_time the amount fo time the key has been held
 * @param aux the hh_aux that contains the list of bools
 */
void kh_update_bool_list(key_index_e index,
                         key_event_type_t type,
                         double held_time,
                         hh_key_aux_t *aux);

/**
 * kh_helping_on_key calls hh_on_key with the correct key and an
 * KEY_PRESSED signal to allow for keys that are still being held
 * to still affect the world even if their key event isn't still occuring
 * Note: not compatible with the concept of held_time, don't refer to it
 * @param aux the key_aux to pass to hh_on_key
 */
void kh_helping_on_key(hh_key_aux_t *aux);

/**
 * Releases the associated memory allocated to the key
 * aux in hungry hungry hippos
 * @param aux the key_aux to be freed
 */
void hh_key_aux_free(hh_key_aux_t *aux);

/**
 * Returns the player stored at the index in the array, or NULL if no
 * hippo is there.
 * @param aux the key_aux storing the array of hippos
 * @param index the index of the hippo in the array
 */
player_t *kh_get_hippo_from_array(hh_key_aux_t *aux, size_t index);

#endif // #ifnded __KEY_HANDLING_H__
