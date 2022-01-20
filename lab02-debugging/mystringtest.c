#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mystring.h"

const int MAX_INPUT_SIZE = 10000;

int main(int argc, char *argv[]) {
    assert(argc == 2 && "usage: mystringtest file");
    char *buf = malloc(MAX_INPUT_SIZE);
    FILE *in = fopen(argv[1], "r");
    assert(in && "file not found");
    buf[fread(buf, sizeof(char), MAX_INPUT_SIZE, in) - 1] = '\0'; 
    fclose(in);
    strarray *result = strsplit(buf);
    for (int i = 0; i < result->length; i++) {
        printf("%d: \"%s\"\n", i, result->data[i]);
        free(result->data[i]);
    }
    free(buf);
    free (result->data);
    free (result);
}