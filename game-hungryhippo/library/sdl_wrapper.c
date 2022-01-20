/**
 * NOTES
 * - "Scene coordinates", abbreviated 'sce', are double x, y coordinates as
 *   recognized by scene and the rest of our physics engine. Unless otherwise
 *   noted, any point of type `vector_t` is in scene coordinates.
 * - "Screen coordinates", abbreviated 'scr', are int x, y coordinates as
 *   recognized by SDL's renderer. Unless otherwise noted, any point of type
 *   `SDL_Point` is in SDL coordinates.
 * - Similarly, there are "sce angles" versus "scr angles". The former are in
 *   radians and increase anticlockwise (reasonable). The latter are in degrees
 *   and increase clockwise (unreasonable).
 */
#include "sdl_wrapper.h"
#include "gfx_aux.h"
#include "sprite.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/**
 * Force the rendering of shape, regardless of whether a body has a gfx_aux, for
 * testing purposes.
 */
const bool ALWAYS_RENDER_SHAPE = false;

const char WINDOW_TITLE[] = "Extremely Hungry Hungry Hippos";
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const double MS_PER_S = 1e3;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window = NULL;

/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer = NULL;

// triggers the program that controls
// your graphics hardware and sets flags
Uint32 RENDER_FLAGS = SDL_RENDERER_ACCELERATED;
Uint32 WINDOW_FLAGS = 0;

/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;

/**
 * Aux object to be passed to the key_handler.
 */
void *key_handler_aux = NULL;

/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;

/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
    if (window == NULL) {
        fprintf(stderr,
                "Fatal error: sdl_wrapper/get_window_center: window not "
                "initialized yet.\n");
        exit(1);
    }
    int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
    assert(width != NULL);
    assert(height != NULL);
    SDL_GetWindowSize(window, width, height);
    vector_t dimensions = {.x = *width, .y = *height};
    free(width);
    free(height);
    return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
    // Scale scene so it fits entirely in the window
    double x_scale = window_center.x / max_diff.x,
           y_scale = window_center.y / max_diff.y;
    return x_scale < y_scale ? x_scale : y_scale;
}

double sdl_sce_to_scr_scale(void) {
    return get_scene_scale(get_window_center());
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
    // Scale scene coordinates by the scaling factor
    // and map the center of the scene to the center of the window
    vector_t scene_center_offset = vec_subtract(scene_pos, center);
    double scale = get_scene_scale(window_center);
    vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
    vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                      // Flip y axis since positive y is down on the screen
                      .y = round(window_center.y - pixel_center_offset.y)};
    return pixel;
}

SDL_Point sdl_sce_to_scr_coord(vector_t scene_pos) {
    vector_t window_center = get_window_center();
    vector_t scr_pos = get_window_position(scene_pos, window_center);
    return (SDL_Point){(int)scr_pos.x, (int)scr_pos.y};
}

double sdl_sce_to_scr_angle(double angle_sce) {
    return -angle_sce * 180 * M_1_PI;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
    switch (key) {
    case SDLK_LEFT:
        return LEFT_ARROW;
    case SDLK_UP:
        return UP_ARROW;
    case SDLK_RIGHT:
        return RIGHT_ARROW;
    case SDLK_DOWN:
        return DOWN_ARROW;
    case SDLK_SLASH:
        return SLASH;
    case SDLK_KP_4:
        return SDL_4;
    case SDLK_KP_1:
        return SDL_1;
    case SDLK_KP_2:
        return SDL_2;
    case SDLK_KP_3:
        return SDL_3;
    case SDLK_KP_5:
        return SDL_5;
    default:
        // Only process 7-bit ASCII characters
        return key == (SDL_Keycode)(char)key ? key : '\0';
    }
}

void sdl_init(vector_t min, vector_t max) {
    // Check parameters
    assert(min.x < max.x);
    assert(min.y < max.y);

    center = vec_multiply(0.5, vec_add(min, max));
    max_diff = vec_subtract(max, center);
    // retutns zero on success else non-zero
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Fatal error initializing SDL: %s\n", SDL_GetError());
    }
    window = SDL_CreateWindow(WINDOW_TITLE,
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH,
                              WINDOW_HEIGHT,
                              WINDOW_FLAGS);

    renderer = SDL_CreateRenderer(window, -1, RENDER_FLAGS);
}

bool sdl_is_done(void) {
    SDL_Event *event = malloc(sizeof(*event));
    assert(event != NULL);
    while (SDL_PollEvent(event)) {
        switch (event->type) {
        case SDL_QUIT:
            free(event);
            return true;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            // Skip the keypress if no handler is configured
            // or an unrecognized key was pressed
            if (key_handler == NULL)
                break;
            char key = get_keycode(event->key.keysym.sym);
            if (key == '\0')
                break;

            uint32_t timestamp = event->key.timestamp;
            if (!event->key.repeat) {
                key_start_timestamp = timestamp;
            }
            key_event_type_t type
                = event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
            double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
            key_handler(key, type, held_time, key_handler_aux);
            break;
        }
    }
    free(event);
    return false;
}

void sdl_clear(void) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
}

void sdl_draw_polygon(list_t *points, rgb_color_t color) {
    // Check parameters
    size_t n = list_size(points);
    assert(n >= 3);
    assert(0 <= color.r && color.r <= 1);
    assert(0 <= color.g && color.g <= 1);
    assert(0 <= color.b && color.b <= 1);

    vector_t window_center = get_window_center();

    // Convert each vertex to a point on screen
    int16_t *x_points = malloc(sizeof(*x_points) * n),
            *y_points = malloc(sizeof(*y_points) * n);
    assert(x_points != NULL);
    assert(y_points != NULL);
    for (size_t i = 0; i < n; i++) {
        vector_t *vertex = list_get(points, i);
        vector_t pixel = get_window_position(*vertex, window_center);
        x_points[i] = pixel.x;
        y_points[i] = pixel.y;
    }

    // Draw polygon with the given color
    filledPolygonRGBA(renderer,
                      x_points,
                      y_points,
                      n,
                      color.r * 255,
                      color.g * 255,
                      color.b * 255,
                      255);
    free(x_points);
    free(y_points);
}

void sdl_show(void) {
    // Draw boundary lines
    vector_t window_center = get_window_center();
    vector_t max = vec_add(center, max_diff),
             min = vec_subtract(center, max_diff);
    vector_t max_pixel = get_window_position(max, window_center),
             min_pixel = get_window_position(min, window_center);
    SDL_Rect *boundary = malloc(sizeof(*boundary));
    boundary->x = min_pixel.x;
    boundary->y = max_pixel.y;
    boundary->w = max_pixel.x - min_pixel.x;
    boundary->h = min_pixel.y - max_pixel.y;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, boundary);
    free(boundary);

    SDL_RenderPresent(renderer);
}

void sdl_render_body(body_t *body) {
    gfx_aux_t *gfx_aux = body_get_gfx(body);
    if (gfx_aux != NULL) {
        sdl_render_gfx(gfx_aux);
    }
    if (gfx_aux == NULL || ALWAYS_RENDER_SHAPE) {
        sdl_draw_polygon(body_get_shape_nocp(body), body_get_color(body));
    }
}

void sdl_render_sprite(sprite_t *sprite) {
    sprite_render(sprite);
}

void sdl_render_gfx(gfx_aux_t *gfx) {
    gfx_aux_render(gfx);
}

void sdl_on_key(key_handler_t handler, void *aux) {
    key_handler = handler;
    key_handler_aux = aux;
}

double time_since_last_tick(void) {
    clock_t now = clock();
    double difference = last_clock
                            ? (double)(now - last_clock) / CLOCKS_PER_SEC
                            : 0.0; // return 0 the first time this is called
    last_clock = now;
    return difference;
}

SDL_Renderer *sdl_get_renderer() {
    if (renderer == NULL) {
        fprintf(stderr,
                "Fatal error: sdl_get_renderer: no renderer in sdl_wrapper\n");
        exit(1);
    }
    return renderer;
}

void sdl_free_all(void) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void sdl_handle_error(char *loc, bool success) {
    if (!success) {
        fprintf(stderr, "SDL Error: %s: %s\n", loc, SDL_GetError());
        exit(1);
    }
}
