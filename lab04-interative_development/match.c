#include "match.h"
#include <stdio.h>

bool match(char *pattern, char *text) {
    if (pattern[0] == '\0' || text[0] == '\0'){
        if (pattern[0] == '\0' && text[0] == '\0' || (pattern[1] == '*' && pattern[2] == '\0')){
            return true;
        }
        return false;
    }
    // Feature 3 and 4
    if(pattern[1] == '*'){
        char same_char = pattern[0];
        if (match(pattern + 2, text)){
            return true;
        }
        if (text[0] == same_char || same_char == '.'){
            if (match(pattern, text + 1)){
                return true;
            }
        }
    }
    else {
        // Feature 1
        if(pattern[0] == text[0]){
            return match(pattern + 1, text + 1);
        }
        
        // Feature 2
        if(pattern[0] ==  '.'){
            return match(pattern + 1, text + 1);
        }
    }
    return false;
}