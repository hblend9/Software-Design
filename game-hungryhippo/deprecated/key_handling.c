#include "key_handling.h"
#include "player.h"
#include "sdl_wrapper.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const double THRESHOLD = 0.0001; // seconds
const int MAX_HIPPOS = 5;

hh_key_aux_t *hh_key_aux_init(game_world_t *game,
                              key_handler_enumed_t key_handler) {
    hh_key_aux_t *aux = malloc(sizeof(hh_key_aux_t));
    aux->player_array = malloc(sizeof(player_t *) * MAX_HIPPOS);
    list_t *hippos = game_world_get_hippos(game);
    // fill in the array of hippos with real hippos
    for (int i = 0; i < list_size(hippos); i++) {
        aux->player_array[i] = (player_t *)list_get(hippos, i);
    }
    // going from the back of the array, fill spots without hippos as null
    for (size_t j = 4; j >= list_size(hippos); j--) {
        aux->player_array[j] = NULL;
    }
    aux->bool_keys = malloc(sizeof(bool) * MAX_SIZE);
    aux->game = game;
    memset(aux->bool_keys, 0, MAX_SIZE);
    aux->already_shot_this_press_array = malloc(sizeof(bool) * MAX_HIPPOS);
    memset(aux->already_shot_this_press_array, 0, MAX_HIPPOS);
    aux->already_switched_this_press_array = malloc(sizeof(bool) * MAX_HIPPOS);
    memset(aux->already_switched_this_press_array, 0, MAX_HIPPOS);

    aux->key_handler = key_handler;
    return aux;
}

// do the same things as key_aux_init but without re-mallocing anything
// needs to be called after worst hippo is removed
hh_key_aux_t *hh_key_aux_reset(game_world_t *game, hh_key_aux_t *aux) {
    list_t *hippos = game_world_get_hippos(game);
    // fill in the array of hippos with real hippos
    for (int i = 0; i < list_size(hippos); i++) {
        aux->player_array[i] = (player_t *)list_get(hippos, i);
        printf("added a hippo to key_aux \n");
    }
    // going from the back of the array, fill spots without hippos as null
    for (size_t j = 4; j >= list_size(hippos); j--) {
        aux->player_array[j] = NULL;
        printf("added a NULL to key_aux \n");
    }
    memset(aux->bool_keys, 0, MAX_SIZE);
    memset(aux->already_shot_this_press_array, 0, MAX_HIPPOS);
    memset(aux->already_switched_this_press_array, 0, MAX_HIPPOS);
    printf("all good in key_handling_reset_aux \n");
    return aux;
}

bool kh_currently_shot(hh_key_aux_t *aux, size_t index) {
    assert((index >= 0) && (index < MAX_SIZE));
    return aux->already_shot_this_press_array[index];
}

bool kh_currently_switched(hh_key_aux_t *aux, size_t index) {
    assert((index >= 0) && (index < MAX_SIZE));
    return aux->already_switched_this_press_array[index];
}

bool kh_currently_pressed(key_index_e index, hh_key_aux_t *aux) {
    assert((index >= 0) && (index < MAX_SIZE));
    return aux->bool_keys[index];
}

bool kh_currently_pressed_key(char key, hh_key_aux_t *aux) {
    key_index_e index = kh_map_key_event(key);
    return kh_currently_pressed(index, aux);
}

// no maps exist in C, very sad
key_index_e kh_map_key_event(char key) {
    key_index_e key_change = -1.0;
    switch (key) {
    case LEFT_ARROW:
        key_change = KEY_LEFT;
        break;
    case RIGHT_ARROW:
        key_change = KEY_RIGHT;
        break;
    case UP_ARROW:
        key_change = KEY_UP;
        break;
    case DOWN_ARROW:
        key_change = KEY_DOWN;
        break;
    case SLASH:
        key_change = KEY_SLASH;
        break;
    case 'a':
        key_change = KEY_A;
        break;
    case 'd':
        key_change = KEY_D;
        break;
    case 'w':
        key_change = KEY_W;
        break;
    case 's':
        key_change = KEY_S;
        break;
    case 'e':
        key_change = KEY_E;
        break;
    case 'h':
        key_change = KEY_H;
        break;
    case 'l':
        key_change = KEY_L;
        break;
    case 'k':
        key_change = KEY_K;
        break;
    case 'j':
        key_change = KEY_J;
        break;
    case 'i':
        key_change = KEY_I;
        break;
    case '1':
        key_change = KEY_1;
        break;
    case SDL_1:
        key_change = KEY_1;
        break;
    case '4':
        key_change = KEY_4;
        break;
    case SDL_4:
        key_change = KEY_4;
        break;
    case '3':
        key_change = KEY_3;
        break;
    case SDL_3:
        key_change = KEY_3;
        break;
    case '2':
        key_change = KEY_2;
        break;
    case SDL_2:
        key_change = KEY_2;
        break;
    case '5':
        key_change = KEY_5;
        break;
    case SDL_5:
        key_change = KEY_5;
        break;
    default:
        key_change = KEY_PASSTHROUGH;
    }
    return key_change;
}

void kh_update_bool_list_char(char key,
                              key_event_type_t type,
                              double held_time,
                              hh_key_aux_t *aux) {
    key_index_e curr_key = kh_map_key_event(key);
    if (curr_key == KEY_PASSTHROUGH) {
        return;
    }
    kh_update_bool_list(curr_key, type, held_time, aux);
}

void kh_update_bool_list(key_index_e index,
                         key_event_type_t type,
                         double held_time,
                         hh_key_aux_t *aux) {
    if (type == KEY_RELEASED) {
        // if key released, set the key to false
        aux->bool_keys[index] = 0;
        //*(bool *)list_get(aux->bool_keys, index) = 0;
        return;
    }
    if (held_time > THRESHOLD) {
        // if key has been pressed for longer than threshold, ignore
        return;
    }
    aux->bool_keys[index] = 1;
    //*(bool *)list_get(aux->bool_keys, index) = 1;
}

void kh_helping_on_key(hh_key_aux_t *aux) {
    for (int i = 0; i < MAX_SIZE; i++) {
        if ((aux->bool_keys[i]) || (i == KEY_SLASH) || (i == KEY_E)
            || (i == KEY_I) || (i == KEY_4) || (i == KEY_DOWN) || (i == KEY_S)
            || (i == KEY_J) || (i == KEY_2)) {
            aux->key_handler(i, KEY_PRESSED, aux);
        }
    }
}

void hh_key_aux_free(hh_key_aux_t *aux) {
    free(aux->bool_keys);
    free(aux->player_array);
    free(aux->already_shot_this_press_array);
    free(aux->already_switched_this_press_array);
    free(aux);
}

player_t *kh_get_hippo_from_array(hh_key_aux_t *aux, size_t index) {
    assert((index >= 0) && (index < MAX_HIPPOS));
    return aux->player_array[index];
}
