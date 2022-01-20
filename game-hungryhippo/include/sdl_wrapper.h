/**
 * USAGE
 *  Main loop will have the following pattern:
 *  ```
 *  // Init the `game_t game`.
 *  sdl_on_key(key_handler, key_aux);
 *  sdl_init(MIN, MAX);
 *  SDL_Renderer *rend = sdl_get_renderer();
 *  while (!sdl_is_done()) {
 *      // Possibly other tick-wise checks such as round-change, end-game, or
 *      // spawn conditions.
 *      dt = time_since_list_tick();
 *      SDL_RenderClear(rend);
 *      game_world_tick(game, dt);
 *      sdl_render_game(game);
 *      SDL_RenderPresent(rend);
 *  }
 *  // Free stuff.
 */

#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include "color.h"
#include "gfx_aux.h"
#include "list.h"
#include "sprite.h"
#include "vector.h"
#include <stdbool.h>

// Values passed to a key handler when the given arrow key is pressed
typedef enum {
    LEFT_ARROW = 1,
    UP_ARROW = 2,
    RIGHT_ARROW = 3,
    DOWN_ARROW = 4,
    SLASH = 5,
    SDL_4 = 6,
    SDL_1 = 7,
    SDL_2 = 8,
    SDL_3 = 9,
    SDL_5 = 10
} arrow_key_t;

/**
 * The possible types of key events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int KeyEventType;
 * #define KEY_PRESSED 0
 * #define KEY_RELEASED 1
 */
typedef enum { KEY_PRESSED, KEY_RELEASED } key_event_type_t;

/**
 * A keypress handler.
 * When a key is pressed or released, the handler is passed its char value.
 * Most keys are passed as their char value, e.g. 'a', '1', or '\r'.
 * Arrow keys have the special values listed above.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 * @param aux object to be passed to key handler
 */
typedef void (*key_handler_t)(char key,
                              key_event_type_t type,
                              double held_time,
                              void *aux);

/**
 * Initializes the SDL window and renderer.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
void sdl_init(vector_t min, vector_t max);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle keypresses.
 *
 * @return true if the window was closed, false otherwise
 */
bool sdl_is_done(void);

/**
 * Clears the screen. Should be called before drawing polygons in each frame.
 */
void sdl_clear(void);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 */
void sdl_show(void);

void sdl_render_sprite(sprite_t *sprite);

void sdl_render_body(body_t *body);

/**
 * Draws a polygon from the given list of vertices and a color.
 *
 * @param points the list of vertices of the polygon
 * @param color the color used to fill in the polygon
 */
void sdl_draw_polygon(list_t *points, rgb_color_t color);

void sdl_render_gfx(gfx_aux_t *gfx);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, key_event_type_t type, double held_time) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 printf("A pressed\n");
 *                 break;
 *             case UP_ARROW:
 *                 printf("UP pressed\n");
 *                 break;
 *         }
 *     }
 * }
 * int main(void) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 * @param aux object to be passed to the handler with each key press
 */
void sdl_on_key(key_handler_t handler, void *aux);

/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);

SDL_Renderer *sdl_get_renderer();

/**
 * For absolute position.
 */
SDL_Point sdl_sce_to_scr_coord(vector_t scene_pos);

double sdl_sce_to_scr_angle(double angle_sce);

/**
 * Free, nay destroy, window and renderer.
 */
void sdl_free_all(void);

/**
 * Get the scale factor that from sceen to screen coordinates.
 * sdl_init must be called first.
 */
double sdl_sce_to_scr_scale(void);

/**
 * Error handling wrapper for SDL interface. In case of an error, the latest SDL
 * error message is printed, so don't do multiple SDL operations before handling
 * errors.
 * @param loc Colon+space-delimited trace of the location of the potential
 * error.
 * @param success Whether the SDL operation was successful or not. If true,
 * nothing happens; if false, print SDL's error message to standard out and exit
 * with nonzero status.
 */
void sdl_handle_error(char *loc, bool success);

#endif // #ifndef __SDL_WRAPPER_H__
