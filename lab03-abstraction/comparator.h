#ifndef __COMPARATOR_H
#define __COMPARATOR_H

#include <stddef.h>

typedef int (*int_comparator_t)(const int a, const int b);
int int_asc(const int a, const int b);
int int_desc(const int a, const int b);

typedef int (*string_comparator_t)(const char *a, const char *b);
/* We don't need to define a string_comparator_t because strcmp is one! */

typedef int (*comparator_t)(const void *a, const void *b);
int int_p_asc(const int *a, const int *b);
int int_p_desc(const int *a, const int *b);

#endif /* __COMPARATOR_H */
