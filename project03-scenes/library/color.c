#include "color.h"
#include <stdio.h>
#include <stdlib.h>

// creates yellow
rgb_color_t rgb_color_init(){
    rgb_color_t random_color = { 
        .r = 0.5,
        .g = 0.5,
        .b = 0.0
    };
    return random_color;
}