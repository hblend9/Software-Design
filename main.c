#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sort.h"

void print_int_array(size_t length, int arr[]) {
    for (int i = 0; i < length; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

void print_int_p_array(size_t length, int *arr[]) {
    for (int i = 0; i < length; i++) {
        printf("%d ", *arr[i]);
    }
    printf("\n");
}

void print_str_array(size_t length, char *arr[]) {
    for (int i = 0; i < length; i++) {
        printf("%s ", arr[i]);
    }
    printf("\n");
}

const int INT_TEST_ARRAY[] = {15, 10, 20, 15, 5, 15, 100, 15, 15, 15, 200, 15, 150};
const char *STR_TEST_ARRAY[] = {"aaaa", "aa", "h", "aaaa", "a", "aaaa", "l", "d", "d", "d", "p", "d", "m"};
const size_t NUM_ELEMENTS = 13;

int *get_int_array() {
    int *test_array = malloc(NUM_ELEMENTS * sizeof(int));
    for (size_t i = 0; i < NUM_ELEMENTS; i++) {
        test_array[i] = INT_TEST_ARRAY[i];
    }
    return test_array;
}

int **get_int_p_array() {
    int **test_array = malloc(NUM_ELEMENTS * sizeof(int *));
    for (size_t i = 0; i < NUM_ELEMENTS; i++) {
        test_array[i] = (int *)&INT_TEST_ARRAY[i];
    }
    return test_array;
}

char **get_str_array() {
    char **test_array = malloc(NUM_ELEMENTS * sizeof(char *));
    for (size_t i = 0; i < NUM_ELEMENTS; i++) {
        test_array[i] = (char *)STR_TEST_ARRAY[i];
    }
    return test_array;
}

#define RUN_FUNC(TYPE, FUNC, ...) do { \
    printf(#FUNC"(" #__VA_ARGS__ "):\n    ");\
    TYPE *arr = get_ ## TYPE ## _array();\
    print_ ## TYPE ## _array(NUM_ELEMENTS, arr);\
    FUNC((void *)arr, __VA_ARGS__);\
    printf("    ");\
    print_ ## TYPE ## _array(NUM_ELEMENTS, arr);\
    free(arr);\
} while(0);

typedef int* int_p;
typedef char* str;

int main(int argc, char *argv[]) {
    RUN_FUNC(int, ascending_int_sort_whole, NUM_ELEMENTS); 

    RUN_FUNC(int, ascending_int_sort, 0, NUM_ELEMENTS); 
    RUN_FUNC(int, ascending_int_sort, 0, 2); 
    RUN_FUNC(int, ascending_int_sort, 0, 4); 

    RUN_FUNC(int, descending_int_sort, 0, NUM_ELEMENTS); 
    RUN_FUNC(int, descending_int_sort, 0, 2); 
    RUN_FUNC(int, descending_int_sort, 0, 4); 

    RUN_FUNC(int, int_sort, 0, NUM_ELEMENTS, int_asc);
    RUN_FUNC(int, int_sort, 0, NUM_ELEMENTS, int_desc);

    RUN_FUNC(str, string_sort, 0, NUM_ELEMENTS, strcmp);

    RUN_FUNC(int_p, sort, 0, NUM_ELEMENTS, (comparator_t)int_p_asc);
    RUN_FUNC(int_p, sort, 0, NUM_ELEMENTS, (comparator_t)int_p_desc);
    RUN_FUNC(str, sort, 0, NUM_ELEMENTS, (comparator_t)strcmp);
}
