#include "text.h"
#include "sdl_wrapper.h"

/*** PRIVATE CONSTS ***/
const char *_FONT_PATHS[TEXT_STYLE_COUNT]
    = {"static/font/UbuntuMono-R.ttf", "static/font/UbuntuMono-B.ttf"};

/*** PRIVATE GLOBALS ***/
int _text_count = 0;

/*** STRUCTURES ***/

struct text_tab {
    SDL_Point topleft; // Top left of table in screen coordinates.
    int row_height; // In pixels (i.e. screen proportions). Also used for font.
    list_t *cols;
    list_t *datas;
    size_t max_rows;
};

typedef struct _text_col {
    int left; // Horizontal position of left in screen coordinates.
    int width;
    text_col_func_t func;
    SDL_Color color;
    TTF_Font *font;
} _text_col_t;

struct text_ln {
    SDL_Point center;
    int height;
    char *str;
    SDL_Color color;
    TTF_Font *font;
    bool removed;
};

/*** PRIVATE PROTOTYPES ***/
void _text_render_cell(char *s,
                       TTF_Font *font,
                       SDL_Color color,
                       SDL_Rect *rect);
_text_col_t *_text_col_init(int left,
                            int width,
                            text_col_func_t func,
                            SDL_Color color,
                            TTF_Font *font);
void _text_col_free(_text_col_t *col);
TTF_Font *_text_init_font(text_style_t style, int height);

/*** DEFINITIONS ***/
void _text_render_cell(char *s,
                       TTF_Font *font,
                       SDL_Color color,
                       SDL_Rect *rect) {
    SDL_Surface *surface = TTF_RenderText_Solid(font, s, color);
    sdl_handle_error("text_render_cell: TTF_RenderText_Solid", surface != NULL);
    SDL_Texture *tex
        = SDL_CreateTextureFromSurface(sdl_get_renderer(), surface);
    sdl_handle_error("_text_render_cell: SDL_CreateTextureFromSurface",
                     tex != NULL);
    sdl_handle_error("_text_render_cell: SDL_RenderCopy",
                     SDL_RenderCopy(sdl_get_renderer(), tex, NULL, rect) == 0);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(tex);
}

void text_tab_render(text_tab_t *tab) {
    size_t row_cnt = tab->max_rows < list_size(tab->datas)
                         ? tab->max_rows
                         : list_size(tab->datas);
    // Loop through cols.
    for (size_t col_idx = 0; col_idx < list_size(tab->cols); col_idx++) {
        _text_col_t *col = list_get(tab->cols, col_idx);
        // Loop through rows.
        for (size_t row_idx = 0; row_idx < row_cnt; row_idx++) {
            char *s = col->func(list_get(tab->datas, row_idx));
            SDL_Rect *rect = malloc(sizeof(SDL_Rect));
            rect->x = col->left;
            rect->y = tab->topleft.y + row_idx * tab->row_height;
            sdl_handle_error("text_tab_render: TTF_SizeText",
                             TTF_SizeText(col->font, s, &(rect->w), &(rect->h))
                                 == 0);
            _text_render_cell(s, col->font, col->color, rect);
            free(rect);
            free(s);
        }
    }
}

text_tab_t *text_tab_init(list_t *datas,
                          size_t max_rows,
                          vector_t topleft,
                          int row_height) {
    // Lazy-load the TTF engine.
    if (!TTF_WasInit()) {
        sdl_handle_error("text_tab_init: TTF_Init", TTF_Init() == 0);
    }

    text_tab_t *tab = malloc(sizeof(text_tab_t));
    assert(tab != NULL);
    tab->topleft = sdl_sce_to_scr_coord(topleft);
    tab->row_height = row_height;
    tab->cols = list_init(1, (free_func_t)_text_col_free);
    tab->datas = datas;
    tab->max_rows = max_rows;
    _text_count++;
    return tab;
}

void text_tab_free(text_tab_t *tab) {
    list_free(tab->cols);
    free(tab);
    _text_count--;

    // Close TTF engine if no more texts left.
    if (_text_count <= 0) {
        TTF_Quit();
    }
}

TTF_Font *_text_init_font(text_style_t style, int height) {
    return TTF_OpenFont(_FONT_PATHS[style], height);
}

void text_tab_add_col(text_tab_t *tab,
                      int width,
                      text_col_func_t func,
                      SDL_Color color,
                      text_style_t style) {
    TTF_Font *font = _text_init_font(style, tab->row_height);
    sdl_handle_error("text_tab_add_col: _text_init_font", font != NULL);
    int left = tab->topleft.x;
    for (size_t i = 0; i < list_size(tab->cols); i++) {
        left += ((_text_col_t *)list_get(tab->cols, i))->width;
    }
    _text_col_t *col = _text_col_init(left, width, func, color, font);
    list_add(tab->cols, col);
}

_text_col_t *_text_col_init(int left,
                            int width,
                            text_col_func_t func,
                            SDL_Color color,
                            TTF_Font *font) {
    _text_col_t *col = malloc(sizeof(_text_col_t));
    assert(col != NULL);
    col->left = left;
    col->width = width;
    col->func = func;
    col->font = font;
    col->color = color;
    return col;
}

void _text_col_free(_text_col_t *col) {
    TTF_CloseFont(col->font);
    free(col);
}

bool text_tab_is_removed(text_tab_t *tab) {
    return false;
}

text_ln_t *text_ln_init(char *str,
                        vector_t center,
                        int height,
                        SDL_Color color,
                        text_style_t style) {
    // Lazy-load the TTF engine.
    if (!TTF_WasInit()) {
        sdl_handle_error("text_tab_init: TTF_Init", TTF_Init() == 0);
    }

    text_ln_t *ln = malloc(sizeof(text_ln_t));
    assert(ln != NULL);
    ln->center = sdl_sce_to_scr_coord(center);
    ln->height = height;
    ln->color = color;
    ln->font = _text_init_font(style, height);
    ln->str = calloc(1, sizeof(char));
    text_ln_update(ln, str);
    ln->removed = false;
    _text_count++;
    return ln;
}

void text_ln_remove(text_ln_t *text_ln) {
    text_ln->removed = true;
}

bool text_ln_is_removed(text_ln_t *text_ln) {
    return text_ln->removed;
}

void text_ln_free(text_ln_t *text_ln) {
    free(text_ln->str);
    TTF_CloseFont(text_ln->font);
    free(text_ln);
    _text_count--;

    // Close TTF engine if no more texts left.
    if (_text_count <= 0) {
        TTF_Quit();
    }
}

void text_ln_render(text_ln_t *text_ln) {
    SDL_Rect *rect = malloc(sizeof(SDL_Rect));
    assert(rect != NULL);
    sdl_handle_error(
        "text_ln_render: TTF_SizeText",
        TTF_SizeText(text_ln->font, text_ln->str, &(rect->w), &(rect->h)) == 0);
    rect->x = text_ln->center.x - rect->w / 2;
    rect->y = text_ln->center.y - rect->h / 2;
    _text_render_cell(text_ln->str, text_ln->font, text_ln->color, rect);
    free(rect);
}

void text_ln_update(text_ln_t *text_ln, char *str) {
    free(text_ln->str);
    text_ln->str = calloc(strlen(str) + 1, sizeof(char));
    assert(text_ln->str != NULL);
    strcpy(text_ln->str, str);
}
