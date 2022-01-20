#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int checkUsage(int argc, char *argv[]){
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(1);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    checkUsage(argc, argv);
    FILE *f = fopen(argv[1], "r");
    if (f == NULL){ // trying to open a file that doesnt exist
        printf("longest: %s: No such file or directory\n", argv[1]);
        return 1;
    }
    // edge case handle later fread will return 0/longest length

    char *longestWord = malloc(101*sizeof(char));
    char *result = malloc(sizeof(char));
    int longestLength = 0; //don't allocate memory for ints (you can but no point)
    char *stringArr = malloc(101*sizeof(char));
    int idx = 0;
    //result can only read one character at a time
    //each time you read it forgets the last char
    //we need an empty array to build string
    //need to also keep track of index/move index up from index 0
    //use strcpy() when the current word is longer than the last word (makes array into string)

    if (result != NULL){
        while (fread(result, 1, 1, f)) {
            if (result[0] == '\n' || result[0] == ' ') {
                if(idx > longestLength){
                    stringArr[idx] = '\0';
                    strcpy(longestWord, stringArr);
                    longestLength = idx;
                }
                idx = 0;//this allows you to just write over the old array instead of reallocating and deallocating memory
            }
            else {
                //add to current array of characters
                stringArr[idx] = result[0]; //or *result since you have one char
                idx++;
            }
        }
        if (longestLength != 0){
            printf("%s\n", longestWord);
        }
    }
    free(result);
    free(stringArr);
    free(longestWord);
    fclose(f);
}