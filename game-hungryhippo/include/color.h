#ifndef __COLOR_H__
#define __COLOR_H__

/**
 * A color to display on the screen.
 * The color is represented by its red, green, and blue components.
 * Each component must be between 0 (black) and 1 (white).
 */
typedef struct {
    float r;
    float g;
    float b;
} rgb_color_t;

/* const rgb_color_t COLOR_BLACK = {0, 0, 0}; */
/* const rgb_color_t COLOR_WHITE = {1, 1, 1}; */
#define COLOR_BLACK \
    (rgb_color_t) { \
        0, 0, 0     \
    }
#define COLOR_WHITE \
    (rgb_color_t) { \
        1, 1, 1     \
    }

#endif // #ifndef __COLOR_H__
