#include <stdlib.h>
#include <stdio.h>
#include "body.h"

#include "list.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

typedef struct list{
    void **data;
    size_t size;
    size_t capacity;
    free_func_t freer;
}list_t;

typedef void (*free_func_t)(void *);

list_t *list_init(size_t initial_size, free_func_t freer){
    list_t *list = malloc(sizeof(list_t));
    void **array_data = malloc(initial_size * sizeof(void*));

    assert(list != NULL);
    assert(array_data != NULL);

    list->data = array_data;
    list->size = 0;
    list->capacity = initial_size;
    list->freer = freer;

    return list;
}

void list_free(list_t *list){
    size_t size = list->size;
    for (int i = 0; i < size; i++){
        list->freer(list->data[i]);
    }
    free(list->data);
    free(list);
}

size_t list_size(list_t *list){
    return list->size;
}

void *list_get(list_t *list, size_t index){
    assert(index < list->size && index>= 0);
    return list->data[index];
}

void *list_remove(list_t *list, size_t index){
    assert(list->size > 0);
    void *temp = list->data[index];
    for (int i = index; i < list->size - 1; i++){
        list->data[i] = list->data[i + 1];
    }
    list->size--;
    return temp;
}

void list_add(list_t *list, void *value){
    assert(value != NULL);
    if (list->size >= list->capacity){
        int prev_size = list->size;
        list_resize(list, prev_size * 2);
    }
    list->data[list->size] = value;
    list->size++;
}

void list_resize(list_t *list, size_t size){
    list->capacity = size;
    void **temp = realloc(list->data, sizeof(void*) * size);
    list->data = temp;
}

void list_replace(list_t *list, int index, void *elem){
    assert(elem != NULL);
    list->data[index] = elem;
}

free_func_t list_get_freer(list_t *list){
    return list->freer;
}