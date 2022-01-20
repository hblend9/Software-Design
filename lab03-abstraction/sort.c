#include "sort.h"

void ascending_int_sort_whole(int arr[], size_t nelements) {
    ascending_int_sort(arr, 0, nelements);
}

void ascending_int_sort(int arr[], size_t lo, size_t hi) {
    int_sort(arr, lo, hi, int_asc);
}

void descending_int_sort(int arr[], size_t lo, size_t hi) {
    int_sort(arr, lo, hi, int_desc);
}

void int_sort(int arr[], size_t lo, size_t hi, int_comparator_t compare) {
    int min_idx = lo;
    for (int i = lo; i < hi - 1; i++) {
        min_idx = i;
        for (int c = i + 1; c < hi; c++) {
            if (compare(arr[c], arr[min_idx]) < 0) { // Just need one if statement for ascending and descending
                min_idx = c;
          }
        }   
        int temp = arr[min_idx];
        arr[min_idx] = arr[i];
        arr[i] = temp;
    }
}

void string_sort(char *arr[], size_t lo, size_t hi, string_comparator_t compare) {
    sort((void *) arr, lo, hi, (comparator_t) compare);
}

void sort(void *arr[], size_t lo, size_t hi, comparator_t compare) {
    int min_idx = lo;
    for (int i = lo; i < hi - 1; i++) {
        min_idx = i;
        for (int c = i + 1; c < hi; c++) {
            if (compare(arr[c], arr[min_idx]) < 0) {
                min_idx = c;
            }
        }
        void *temp = arr[min_idx];
        arr[min_idx] = arr[i];
        arr[i] = temp;
   }
}
