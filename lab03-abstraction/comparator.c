#include <string.h>
#include "comparator.h"

int int_asc(const int a, const int b) {
    return a - b;
}

int int_desc(const int a, const int b) {
    return b - a;
}

int int_p_asc(const int *a, const int *b) {
    return *a - *b;
}

int int_p_desc(const int *a, const int *b) {
    return *b - *a;
}
