#include <stddef.h>
#include "vector.h"
#include "vec_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


typedef struct vec_list  {
  vector_t **array;
  size_t size;
  size_t capacity;
} vec_list_t;


vec_list_t *vec_list_init(size_t initial_size) {
    vec_list_t *list2 = malloc(sizeof(vec_list_t));
    assert(list2 != NULL && "The required memory was not allocated"); //only get the message if it it doesn't work
    vector_t **list = malloc(initial_size * sizeof(vector_t *));
    assert(list2->array != NULL && "The required memory was not allocated");
    list2->array = list;
    list2->capacity = initial_size;
    list2->size = 0;
    return list2;
}


void vec_list_free(vec_list_t *list) {
    if (list->size > 0) {
        for (int i = 0; i < (int) list->capacity; i++) {
            free(list->array[i]);
        }
    }
    free(list->array);
    free(list);
}


size_t vec_list_size(vec_list_t *list){
    return list->size;
}


vector_t *vec_list_get(vec_list_t *list, size_t index){
    assert(list->size > index && "The index is not valid given the list's current size");
    return list->array[index];
}


void vec_list_add(vec_list_t *list, vector_t *value){
    size_t size = vec_list_size(list);
    assert(size < list->capacity && "The list doesn't have remaining space");
    assert(value != NULL && "The value is null");
    list->array[size] = value;
    list->size += 1;
}


vector_t *vec_list_remove(vec_list_t *list){
    size_t size = vec_list_size(list);
    assert(size > 0 && "The list is empty");
    vector_t *remove_vector = list->array[size - 1];
    list->array[size - 1] = NULL;
    list->size -= 1;
    return remove_vector;
}