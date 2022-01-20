#include "list.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

const double LIST_RESIZE_FACTOR = 2.0;

// Private function prototypes.
/**
 * Dynamically resize capacity of 'list' by 'LIST_RESIZE_FACTOR'.
 * @param list the list to be resized
 */
void list_resize_if_needed(list_t *list);

typedef struct list {
    size_t size;
    size_t capacity;

    free_func_t freer;

    void **elements;
} list_t;

list_t *list_init(size_t initial_size, free_func_t freer) {
    list_t *l = malloc(sizeof(list_t));
    assert(l != NULL);

    l->size = 0;
    l->capacity = initial_size;
    l->elements = calloc(initial_size, sizeof(void *));
    assert(l->elements != NULL);

    l->freer = freer;

    return l;
}

void list_free(list_t *list) {
    // If freer is NULL, do not free each element.
    if (list->freer != NULL) {
        for (size_t i = 0; i < list_size(list); i++) {
            list->freer(list->elements[i]);
        }
    }
    free(list->elements);
    free(list);
}

size_t list_size(list_t *list) {
    assert(list != NULL);
    return list->size;
}

void *list_get(list_t *list, size_t index) {
    assert(index >= 0 && index < list->size);

    return list->elements[index];
}

void *list_remove(list_t *list, size_t index) {
    assert(index >= 0 && index < list->size);

    void **elems = list->elements;
    void *e = elems[index];

    memmove(elems + index,
            elems + index + 1,
            sizeof(void *) * ((list->size--) - index - 1));

    return e;
}

void list_add(list_t *list, void *value) {
    assert(value != NULL);

    list_resize_if_needed(list);

    list->elements[list->size++] = value;
}

void list_resize_if_needed(list_t *list) {
    if (list_size(list) >= list->capacity) {
        size_t new_capacity = (size_t)(list->capacity * LIST_RESIZE_FACTOR);

        void **new_elems = malloc(sizeof(void *) * new_capacity);
        assert(new_elems != NULL);

        memcpy(new_elems, list->elements, list_size(list) * sizeof(void *));
        free(list->elements);

        list->elements = new_elems;
        list->capacity = new_capacity;
    }
}

list_t *list_copy(list_t *list, copy_func_t copier) {
    size_t size = list->size;

    list_t *list_c = list_init(size, list->freer);

    if (copier != NULL) {
        for (size_t i = 0; i < size; i++) {
            list_add(list_c, copier(list_get(list, i)));
        }
    } else {
        for (size_t i = 0; i < size; i++) {
            list_add(list_c, list_get(list, i));
        }
    }

    return list_c;
}
