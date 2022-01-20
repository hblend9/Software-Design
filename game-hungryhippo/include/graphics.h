#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

/*** DEPENDENCIES ***/
typedef struct list list_t;
typedef struct vector vector_t;

/*** TYPES ***/

/**
 * A graphical layer that can render 'body's, 'text_tab's, and 'text_box'es.
 */
typedef struct graphics graphics_t;

/*** INTERFACE ***/

/**
 * Create a graphical layer and initialize SDL behind the scenes.
 * This is a singleton; initializing multiple graphics without freeing first
 * causes a fatal error.
 */
graphics_t *graphics_init(vector_t dims);

/**
 * Free the graphics layer.
 * Don't free any renderable objects, that's the job of the client.
 */
void graphics_free(graphics_t *graphics);

/**
 * Render all graphics via sdl.
 */
void graphics_render(graphics_t *graphics);

/**
 * Registers a group of bodies to render from each tick.
 * Bodies are rendered in the ordered added, hence later will be rendered on top
 * of ealier bodies.
 */
void graphics_add_bodies(graphics_t *graphics, list_t *bodies);

/**
 * Register a group of text tabs to render from each tick.
 */
void graphics_add_text_tabs(graphics_t *graphics, list_t *text_tabs);

/**
 * Register a group of text lines to render from each tick.
 */
void graphics_add_text_lns(graphics_t *graphics, list_t *text_lns);

#endif // #ifndef __GRAPHICS_H__
