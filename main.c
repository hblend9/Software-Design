#include <stdio.h>
#include <stdlib.h>
#include "match.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <needle> <haystack>\n", argv[0]);
        exit(1);
    }

    char *needle = argv[1]; 
    char *haystack = argv[2]; 

    if (match(needle, haystack)) {
        printf("match\n");
        return 0;
    }
    else {
        printf("no match\n");
        return 1;
    }
}
