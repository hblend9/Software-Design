#include <stdlib.h>
#include <string.h>
#include <stdio.h>


char *string_reverse(char *s) {
    char *str = malloc((strlen(s) + 1) * sizeof(char));
    size_t x = strlen(s);
    for (size_t i = 0; i < x; i++){
        str[i] = s[x - i -1];
    }
    str[x] = '\0';
    return str;
}

int main(int argc, char *argv[]) {
    char *rev1 = string_reverse("aaaaaaaaaa");
    printf("%s\n", rev1);
    free(rev1);
    char *rev2 = string_reverse("bbbbbbbbb");
    printf("%s\n", rev2);
    free(rev2);
}