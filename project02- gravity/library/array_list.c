#include "array_list.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

typedef struct array_list{
    void **data;
    size_t size;
    size_t capacity;
}array_list_t;

array_list_t *array_list_init(size_t initial_size)
{
    array_list_t *list = malloc(sizeof(array_list_t));
    void **array_data = malloc(initial_size * sizeof(void*));

    assert(list != NULL);
    assert(array_data != NULL);

    list->data = array_data;
    list->size = 0;
    list->capacity = initial_size;
    return list;
}

void array_list_free(array_list_t *list){
    int size = list->size;
    for (int i = 0; i < size; i++){
        free(list->data[i]);
    }
    free(list->data);
    free(list);
}

size_t array_list_size(array_list_t *list){
    return list->size;
}

void *array_list_get(array_list_t *list, size_t index){
    assert(index < list->size);
    return list->data[index];
}

void array_list_resize(array_list_t *list, size_t size){
    list->capacity = size;
    void **temp = realloc(list->data, sizeof(void*) * size);
    list->data = temp;
}

void array_list_add(array_list_t *list, void *value){
    assert(value != NULL);
    if (list->size >= list->capacity){
        int prev_size = list->size;
        array_list_resize(list, prev_size * 2);
    }
    list->data[list->size] = value;
    list->size++;
}

/** Removes first element of the list.
*/
void *array_list_remove(array_list_t *list){
    assert(list->size > 0);
    void *temp = list->data[0];
    for (int i = 0; i < list->size - 1; i++){
        list->data[i] = list->data[i + 1];
    }
    list->size--;
    return temp;
}

void array_list_replace(array_list_t *list, int i, vector_t *vector)
{
    assert(vector != NULL);
    list->data[i] = vector;
}

