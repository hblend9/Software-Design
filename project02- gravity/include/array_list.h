#ifndef __ARRAY_LIST_H__
#define __ARRAY_LIST_H__

#include <stddef.h>
#include "vector.h"

/** A growable array list of stars.
 * Declares struct Arraylist type
 * Makes ArrayList an alias for "struct List"
*/
typedef struct array_list array_list_t;

/**
 * Initializes an empty array list.
 * 
 * @param initial_size the number of list elements to allocate space for
 * @return a pointer to the list
*/
array_list_t *array_list_init(size_t initial_size);

/**
 * Frees the memory for the list and its elements.
 *
 * @param list a pointer to a list returned from vec_list_init()
 */
void array_list_free(array_list_t *list);

/**
 * Gets the size of a list (the number of occupied elements).
 * Note that this is NOT the list's capacity.
 *
 * @param list a pointer to a list returned from vec_list_init()
 * @return the number of vectors in the list
 */
size_t array_list_size(array_list_t *list);

/**
 * Will resize the amount of memory allocated to the array list
 *
 * @param list a pointer to a list returned from vec_list_init()
 * @param size the new capacity for the list
 * @return the number of vectors in the list
 */
void array_list_resize(array_list_t *list, size_t size);

/**
 * Gets the element at a given index in a list.
 * Asserts that the index is valid, given the list's current size.
 *
 * @param list a pointer to a list returned from array_list_init()
 * @param index an index in the list (the first element is at 0)
 * @return the vector at the given index
 */
void *array_list_get(array_list_t *list, size_t index);

/**
 * Appends an element to the end of a list.
 * Asserts that the list has has remaining space,
 * and that the value being added is not NULL.
 *
 * @param list a pointer to a list returned from vec_list_init()
 * @param value the vector to add to the end of the list
 */
void array_list_add(array_list_t *list, void *value);

/**
 * Removes the element at the end of a list and returns it.
 * Asserts that the list is nonempty.
 *
 * @param list a pointer to a list returned from vec_list_init()
 * @return the vector at the end of the list
 */
void *array_list_remove(array_list_t *list);

void array_list_replace(array_list_t *list, int i, vector_t *vector);

#endif // #ifndef __ARRAY_LIST_H__