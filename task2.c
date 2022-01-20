#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char **string_blanks(char *s) {
    char **arr = malloc(strlen(s) * sizeof(char *)); //sizeof char takes care of the pointers to the strings
    for (int i = 0; i < strlen(s); i++){
        arr[i] = malloc((strlen(s) + 1) * sizeof(char));
        strcpy(arr[i], s);
        arr[i][i] = '_';
    }
    return arr;
}

int main(int argc, char *argv[]) {
    char **blanks = string_blanks("Adam");
    for (int i = 0; i < strlen("Adam"); i++) {
        printf("%s ", blanks[i]);
        free(blanks[i]);
    }
    printf("\n");
    free(blanks);
}