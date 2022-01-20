#ifndef __TEXT_H__
#define __TEXT_H__

#include "list.h"
#include "stdbool.h"
#include "vector.h"
#include <SDL2/SDL_ttf.h>

/**
 * Text formatting styles.
 */
typedef enum {
    TEXT_STYLE_NORMAL,
    TEXT_STYLE_BOLD,
    TEXT_STYLE_COUNT
} text_style_t;

/*** TEXT_TAB INTERFACE ***/

/**
 * A table of text.
 * Columns are functions of rows, so to speak.
 * Formatting is constant along columns, auxiliary data is constant along rows.
 * Nothing advanced like padding and margins.
 */
typedef struct text_tab text_tab_t;

/**
 * A type of function that takes the row's data and returns the string that
 * appears in the given column of the table.
 * NOTICE: The return string of this function must be allocated on the heap via
 * malloc/calloc, for it will be freed after each rendering cycle.
 */
typedef char *(*text_col_func_t)(void *);

/**
 * Initialize an empty table.
 * Also load the TTF engine if needed (no need to do this yourself, though doing
 * is okay too).
 * @param datas List of data, each element of which represents a row and parsed
 * into column via the column functions. Note that this list is NOT freed by
 * 'text_tab_free'.
 * @param max_rows Maximum number of rows. If the datas list ever grows beyond
 * this number, then the table will just be truncated at 'max_rows' and no error
 * is raised.
 * @param topleft Position of top left corner in scene coordinates.
 * @param row_height Uniform height of rows in pixels. Translates to font size.
 */
text_tab_t *
text_tab_init(list_t *datas, size_t max_rows, vector_t topleft, int row_height);

/**
 * Free a table but NOT its data.
 * Also close the TTF engine only if there are no more tables in existence :).
 */
void text_tab_free(text_tab_t *tab);

/**
 * Add a column to the right of a table.
 * @param func Function that takes row data and returns the column string.
 * @param width Width of the column in pixels.
 * @param color Text color.
 * @param style A text_style_t text style.
 */
void text_tab_add_col(text_tab_t *tab,
                      int width,
                      text_col_func_t func,
                      SDL_Color color,
                      text_style_t style);

/**
 * Render the table of text with SDL. Requires SDL to have started the text
 * rendering engine first.
 */
void text_tab_render(text_tab_t *tab);

/**
 * Return whether a text_tab has been marked for removal.
 * (Note that at this time text_tab_remove has not been implemented because
 * there has been no need for it in this project; thus this function always
 * returns false.)
 */
bool text_tab_is_removed(text_tab_t *tab);

/*** TEXT_LN INTERFACE ***/

/**
 * A single centered line of text.
 */
typedef struct text_ln text_ln_t;

/**
 * Create a new renderable line of text from 'str'. The string is copied
 * internally, not modifed and should be freed by the client.
 */
text_ln_t *text_ln_init(char *str,
                        vector_t center,
                        int height,
                        SDL_Color color,
                        text_style_t style);

/**
 * Mark the text_ln for removal. This is preferred over text_ln_free if you have
 * a deferred removal system.
 */
void text_ln_remove(text_ln_t *text_ln);

/**
 * Return whether the text_ln is marked for removal.
 */
bool text_ln_is_removed(text_ln_t *text_ln);

/**
 * Free the text_ln.
 */
void text_ln_free(text_ln_t *text_ln);

/**
 * Render the line of text.
 */
void text_ln_render(text_ln_t *text_ln);

/**
 * Update the text.
 */
void text_ln_update(text_ln_t *text_ln, char *str);

#endif // #ifndef __TEXT_H__
