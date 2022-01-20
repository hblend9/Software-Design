#ifndef __LIST_H__
#define __LIST_H__

#include <stddef.h>

/**
 * A growable array of pointers.
 * Can store values of any pointer type (e.g. vector_t*, body_t*).
 * The list automatically grows its internal array when more capacity is needed.
 */
typedef struct list list_t;

/**
 * A function that can be called on list elements to release their resources.
 * Examples: free, body_free
 */
typedef void (*free_func_t)(void *);

/**
 * A function that can be called on list elements to copy their resources.
 */
typedef void *(*copy_func_t)(void *);

/**
 * A function that parses a string into an anything.
 */
typedef void *(*parse_record_func_t)(const char *);

/**
 * Allocates memory for a new list with space for the given number of elements.
 * The list is initially empty.
 * Asserts that the required memory was allocated.
 *
 * @param initial_size the number of elements to allocate space for
 * @param freer if non-NULL, a function to call on elements in the list
 *   in list_free() when they are no longer in use
 * @return a pointer to the newly allocated list
 */
list_t *list_init(size_t initial_size, free_func_t freer);

list_t *list_init_from_path(const char *file_path,
                            free_func_t freer,
                            parse_record_func_t parser);

/**
 * Releases the memory allocated for a list. Does not free each element if freer
 * is NULL.
 *
 * @param list a pointer to a list returned from list_init()
 */
void list_free(list_t *list);

/**
 * Gets the size of a list (the number of occupied elements).
 * Note that this is NOT the list's capacity.
 *
 * @param list a pointer to a list returned from list_init()
 * @return the number of elements in the list
 */
size_t list_size(const list_t *list);

/**
 * Gets the element at a given index in a list.
 * Asserts that the index is valid, given the list's current size.
 *
 * @param list a pointer to a list returned from list_init()
 * @param index an index in the list (the first element is at 0)
 * @return the element at the given index, as a void*
 */
void *list_get(const list_t *list, size_t index);

/**
 * Removes the element at a given index in a list and returns it,
 * moving all subsequent elements towards the start of the list.
 * Asserts that the index is valid, given the list's current size.
 * Returns NULL if the current size is 0.
 *
 * @param list a pointer to a list returned from list_init()
 * @return the element at the given index in the list
 */
void *list_remove(list_t *list, size_t index);

/**
 * Appends an element to the end of a list.
 * If the list is filled to capacity, resizes the list to fit more elements
 * and asserts that the resize succeeded.
 * Also asserts that the value being added is non-NULL.
 *
 * @param list a pointer to a list returned from list_init()
 * @param value the element to add to the end of the list
 */
void list_add(list_t *list, void *value);

/**
 * Copies a list to a new struct.
 * Copies each element of the old list using the copier function.
 *
 * @param list a pointer to a list returned from list_init()
 * @param copier if non-NULL, a function to call on elements in the list
 *  to copy them to the new list, otherwise copies the pointer to each element.
 * @return a pointer to the newly copied list
 */
list_t *list_copy(list_t *list, copy_func_t copier);

/**
 * Reverse a list in-place.
 */
void list_reverse(list_t *list);

#endif // #ifndef __LIST_H__
