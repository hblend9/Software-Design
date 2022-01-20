#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//break up into helper functions
//possibly pull out malloc
//or wc

int checkUsage(int argc, char *argv[]){
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(1);
    }
    return 0;
}

int main(int argc, char *argv[]) { //char *__[] treat as an array of strings and the other is an array of characters
    checkUsage(argc, argv);
    FILE *f = fopen(argv[1], "r");
    if (f == NULL){
        printf("wc: %s: No such file or directory\n", argv[1]);
        return 1;
    }

    int num_lines = 0;
    char *result = malloc(sizeof(char)); //not using result as string so you don't need x2
    if (result == NULL){
        return 1;
    }
    while (fread(result, 1, 1, f)) {
        if (result[0] == '\n') {
            num_lines++;
        }
    }
    printf("%d %s\n", num_lines, argv[1]);
    free(result);
    fclose(f);
}