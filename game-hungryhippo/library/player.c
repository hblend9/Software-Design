#include "player.h"
#include "ehhh.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "sprite.h"
#include "text.h"
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const int _TAB_ROWHEIGHT = 15;
const text_style_t _TAB_STYLE = TEXT_STYLE_BOLD;
const SDL_Color _TAB_COLOR = {0x00, 0x2b, 0x36, 0xfa};
const int _MAX_POINTS = 9999;
const int _MAX_POWERUPS = 99;

/*** PRIVATE CONSTANTS ***/

const char *PLAYER_PATH_SHAPE_CHILLING_BODY = "static/hippo/shape-chilling.csv";
const char *PLAYER_PATH_SHAPE_EATING_BODY = "static/hippo/shape-eating.csv";
const char *PLAYER_PATH_SHAPE_MOUTH = "static/hippo/shape-mouth.csv";
const char *PLAYER_PATH_FMT_SPRITE_CHILLING
    = "static/hippo/hippo_chilling_%1zu.png";
const char *PLAYER_PATH_FMT_SPRITE_EATING
    = "static/hippo/hippo_eating_%1zu.png";
// Factor by which to scale player's sprite and polygon when loaded from file.
const double PLAYER_GFX_SCALE = 1.0 / 3.5;
const double _PLAYER_MOUTH_DILATION = 1.3;
// Offset from mouth center to shooted ball initial position.
const double _PLAYER_SHOOT_OFFSET = 25;

// Player default angular speed around arena in radians / second.
const double PLAYER_DEFAULT_ANGULAR_SPEED = M_PI_4;
// Player default linear shoot speed in "pixels" / second.
const double PLAYER_DEFAULT_SHOOT_SPEED = 500;

/*** PRIVATE PROTOTYPES ***/

/**
 * Make a new body based on the player's index number. Sets the body info to
 * be 'player'.
 */
body_t *player_make_player_body(player_t *player, size_t idx);

// Table column functions.
char *_player_tab_idx_col(body_t *player_body);
char *_player_tab_points_col(body_t *player_body);
char *_player_tab_powerups_col(body_t *player_body);

/*** STRUCTS WITH PRIVATE FIELDS ***/
struct player {
    size_t idx;
    player_state_e state;

    body_t *body;
    list_t **hippo_body_shape;
    list_t **hippo_mouth_shape;
    vector_t origin;

    list_t *powerups;
    size_t powerup_idx;
    short points;

    double speed_factor;
    double angle_modifier;

    bool removed;
};

/*** DEFINITIONS OF PUBLIC FUNCTIONS ***/

player_t *player_init(size_t idx, vector_t origin) {
    player_t *player = malloc(sizeof(player_t));
    player->idx = idx;
    player->state = PLAYER_CHILLING;

    player->body = player_make_player_body(player, idx);
    body_set_centroid(player->body, origin);
    player->origin = origin;

    player->hippo_body_shape = malloc(sizeof(list_t *));
    player->hippo_mouth_shape = malloc(sizeof(list_t *));
    *(player->hippo_body_shape)
        = body_get_shape_alt(player->body, PLAYER_HIPPO_BODY_NOT_EATING_IDX);
    *(player->hippo_mouth_shape)
        = body_get_shape_alt(player->body, PLAYER_HIPPO_MOUTH_IDX);

    player->powerups = list_init(1, (free_func_t)powerup_free);
    player->powerup_idx = 0;
    player->points = 0;
    player->removed = false;

    player->speed_factor = 1;
    player->angle_modifier = 0;

    return player;
}

void player_free(player_t *player) {
    list_free(player->powerups);
    free(player->hippo_body_shape);
    free(player->hippo_mouth_shape);
    free(player);
}

size_t player_get_idx(player_t *player) {
    assert(player != NULL);
    return player->idx;
}

body_t *player_get_body(player_t *player) {
    return player->body;
}

list_t *player_get_powerups(player_t *player) {
    return player->powerups;
}

powerup_t *player_get_powerup(player_t *player, size_t idx) {
    if (idx < 0 || idx >= list_size(player->powerups)) {
        return NULL;
    }
    return list_get(player->powerups, idx);
}

powerup_t *player_remove_powerup(player_t *player, size_t idx) {
    return list_remove(player->powerups, idx);
}

player_state_e player_get_state(player_t *player) {
    return player->state;
}

void player_set_state(player_t *player, player_state_e state) {
    player->state = state;
    switch (state) {
    case PLAYER_EATING:
        *player->hippo_body_shape
            = body_get_shape_alt(player_get_body(player),
                                 PLAYER_HIPPO_BODY_EATING_IDX);
        body_set_shape_main(player->body, PLAYER_HIPPO_BODY_EATING_IDX);
        gfx_aux_set_sprite(body_get_gfx(player->body), PLAYER_SPRITE_EATING);
        break;
    case PLAYER_CHILLING:
        *player->hippo_body_shape
            = body_get_shape_alt(player_get_body(player),
                                 PLAYER_HIPPO_BODY_NOT_EATING_IDX);
        body_set_shape_main(player->body, PLAYER_HIPPO_BODY_NOT_EATING_IDX);
        gfx_aux_set_sprite(body_get_gfx(player->body), PLAYER_SPRITE_CHILLING);
        break;
    default:
        break;
    }
}

void player_add_powerup(player_t *player, powerup_t *powerup) {
    list_add(player->powerups, powerup);
}

void player_add_timer_id(player_t *player, SDL_TimerID timer_id) {
    assert(false && "deprecated");
}

void player_activate_powerup(player_t *player, ehhh_t *ehhh) {
    if (list_size(player->powerups) > 0) {
        powerup_t *powerup = player_get_curr_powerup(player);
        powerup_activate(powerup, ehhh, player);
        powerup_free(powerup);
    }
}

powerup_t *player_get_curr_powerup(player_t *player) {
    powerup_t *p = (powerup_t *)list_remove(player->powerups,
                                            player_get_powerup_idx(player));
    player_inc_powerup_idx(player, -1);
    return p;
}

size_t player_get_powerup_idx(player_t *player) {
    return player->powerup_idx;
}

void player_inc_powerup_idx(player_t *player, size_t amount) {
    size_t np = list_size(player->powerups);
    if (np > 0) {
        player->powerup_idx
            = (player->powerup_idx + amount) % list_size(player->powerups);
    }
}

void player_add_points(player_t *player, short points) {
    player->points += points;
}

short player_get_points(player_t *player) {
    return player->points;
}

void player_remove(player_t *player) {
    player->removed = true;
    body_remove(player_get_body(player));
}

list_t **player_get_hippo_body_shape(player_t *player) {
    return player->hippo_body_shape;
}

list_t **player_get_hippo_mouth_shape(player_t *player) {
    return player->hippo_mouth_shape;
}

bool player_is_eating(player_t *player) {
    return player_get_state(player) == PLAYER_EATING;
}

bool player_is_removed(player_t *player) {
    return player->removed;
}

void player_add_speed_factor(player_t *player, double factor) {
    player->speed_factor += factor;
}

void player_set_speed_factor(player_t *player, double factor) {
    player->speed_factor = factor;
}

double player_get_speed_factor(player_t *player) {
    return player->speed_factor;
}

void player_set_angle_modifier(player_t *player, double range) {
    player->angle_modifier = range;
}

void player_add_angle_modifier(player_t *player, double range) {
    player->angle_modifier += range;
}

double player_get_angle_modifier(player_t *player) {
    return player->angle_modifier;
}

vector_t player_get_shoot_location(player_t *player, ehhh_t *ehhh) {
    vector_t a_center = ehhh_get_center(ehhh);
    vector_t b_center
        = polygon_centroid(*(player_get_hippo_mouth_shape(player)));

    vector_t p = vec_normalize(vec_subtract(a_center, b_center));

    p = vec_add(vec_multiply(_PLAYER_SHOOT_OFFSET, p), b_center);

    return p;
}

vector_t player_get_shoot_velocity(player_t *player, ehhh_t *ehhh) {
    vector_t a_center = ehhh_get_center(ehhh);
    vector_t b_center = body_get_centroid(player_get_body(player));

    vector_t hippo_to_center = vec_normalize(vec_subtract(a_center, b_center));

    return vec_multiply(BALL_SHOOT_SPEED * player_get_speed_factor(player),
                        hippo_to_center);
}

text_tab_t *player_create_tab(list_t *player_bodies, vector_t topleft) {
    text_tab_t *tab = text_tab_init(player_bodies,
                                    list_size(player_bodies),
                                    topleft,
                                    _TAB_ROWHEIGHT);
    text_tab_add_col(tab,
                     25,
                     (text_col_func_t)_player_tab_idx_col,
                     _TAB_COLOR,
                     _TAB_STYLE);
    text_tab_add_col(tab,
                     50,
                     (text_col_func_t)_player_tab_points_col,
                     _TAB_COLOR,
                     TEXT_STYLE_NORMAL);
    text_tab_add_col(tab,
                     90,
                     (text_col_func_t)_player_tab_powerups_col,
                     _TAB_COLOR,
                     TEXT_STYLE_NORMAL);
    return tab;
}

/*** DEFINITIONS OF PRIVATE FUNCTIONS ***/

char *_player_tab_idx_col(body_t *player_body) {
    player_t *player = body_get_info(player_body);
    const char *fmt = "P%1zu";
    char *s = calloc(strlen(fmt) + /*idx*/ 1 + /*'\0'*/ 1, sizeof(char));
    if (sprintf(s, fmt, player_get_idx(player)) < 0) {
        fprintf(stderr,
                "player.c: _player_tab_idx_col: Error calling sprintf.\n");
        exit(1);
    }
    return s;
}

char *_player_tab_points_col(body_t *player_body) {
    player_t *player = body_get_info(player_body);
    short points = player_get_points(player);
    // We assume max 9999 points.
    if (points > _MAX_POINTS) {
        points = _MAX_POINTS;
    }
    const char *fmt = "%0+5hd";
    char *s = calloc(strlen(fmt) + /*+points*/ 6 + /*'\0'*/ 1, sizeof(char));
    if (sprintf(s, fmt, player_get_points(player)) < 0) {
        fprintf(stderr,
                "player.c: _player_tab_points_col: Error calling sprintf.\n");
        exit(1);
    }
    return s;
}

char *_player_tab_powerups_col(body_t *player_body) {
    player_t *player = body_get_info(player_body);
    size_t pu_count = list_size(player_get_powerups(player));
    size_t pu_idx = player_get_powerup_idx(player);
    char *pu_str = powerup_str(player_get_powerup(player, pu_idx));
    if (pu_count > _MAX_POWERUPS) {
        pu_count = _MAX_POWERUPS;
    }
    const char *fmt = "%zu/%zu>%s";
    char *s = calloc(strlen(fmt) + /*#/#>*/ 10 + strlen(pu_str) + /*'\0'*/ 1,
                     sizeof(char));
    if (sprintf(s, fmt, pu_count == 0 ? 0 : pu_idx + 1, pu_count, pu_str) < 0) {
        fprintf(stderr,
                "player.c: _player_tab_powerups_col: Error calling sprintf.\n");
        exit(1);
    }
    free(pu_str);
    return s;
}

body_t *player_make_player_body(player_t *player, size_t idx) {
    // assert(PLAYER_NUM_SHAPES == body_get_num_shapes(body));
    if (idx > 4 || idx < 0) {
        idx = 1;
    }

    // Shapes
    // Initialize so that the hippo's head points due north.

    // Chilling body
    list_t *bd_chilling
        = polygon_init_from_path(PLAYER_PATH_SHAPE_CHILLING_BODY);
    polygon_scr_to_sce(bd_chilling);
    polygon_scale(bd_chilling, PLAYER_GFX_SCALE);

    // Eating body
    list_t *bd_eating = polygon_init_from_path(PLAYER_PATH_SHAPE_EATING_BODY);
    polygon_scr_to_sce(bd_eating);
    polygon_scale(bd_eating, PLAYER_GFX_SCALE);
    // Flush botright with chilling body.
    polygon_translate(bd_eating,
                      vec_subtract(polygon_botright(bd_chilling),
                                   polygon_botright(bd_eating)));

    // Mouth
    list_t *m = polygon_init_from_path(PLAYER_PATH_SHAPE_MOUTH);
    polygon_scr_to_sce(m);
    polygon_scale(m, PLAYER_GFX_SCALE);
    // Make mouth flush topleft with eating body.
    polygon_translate(
        m,
        vec_subtract(polygon_topleft(bd_eating), polygon_topleft(m)));
    // Then center.
    polygon_translate(
        m,
        vec_multiply(
            vec_subtract(polygon_centroid(bd_eating), polygon_centroid(m)).x,
            E1));
    // Then dilate mouth slightly so that it contacts balls before collision
    // shape.
    polygon_scale(m, _PLAYER_MOUTH_DILATION);

    // Add shapes to list.
    // Must go in order of:
    // PLAYER_HIPPO_BODY_NOT_EATING_IDX,
    // PLAYER_HIPPO_BODY_EATING_IDX,
    // PLAYER_HIPPO_MOUTH_IDX,
    list_t *shapes = list_init(PLAYER_NUM_SHAPES, (free_func_t)list_free);
    list_add(shapes, bd_chilling);
    list_add(shapes, bd_eating);
    list_add(shapes, m);

    // Sprites
    char *sprite_path;

    // Init chilling sprite.
    sprite_path
        = calloc(strlen(PLAYER_PATH_FMT_SPRITE_CHILLING) + 2, sizeof(char));
    assert(sprite_path != NULL);
    sprintf(sprite_path, PLAYER_PATH_FMT_SPRITE_CHILLING, idx);
    sprite_t *sprite_chilling
        = sprite_init(sprite_path, PLAYER_GFX_SCALE, VEC_ZERO);
    free(sprite_path);

    // Init eating sprite.
    // Chill-to-eat sprite offset
    // = -centroid_to_center(chill) + (eat - chill) + centroid_to-center(eat).
    vector_t chill_to_eat_offset
        = vec_subtract(vec_add(vec_subtract(polygon_centroid(bd_eating),
                                            polygon_centroid(bd_chilling)),
                               polygon_centroid_to_center(bd_eating)),
                       polygon_centroid_to_center(bd_chilling));
    sprite_path
        = calloc(strlen(PLAYER_PATH_FMT_SPRITE_EATING) + 2, sizeof(char));
    assert(sprite_path != NULL);
    sprintf(sprite_path, PLAYER_PATH_FMT_SPRITE_EATING, idx);
    sprite_t *sprite_eating
        = sprite_init(sprite_path, PLAYER_GFX_SCALE, chill_to_eat_offset);
    free(sprite_path);

    // Add sprites to list.
    // This MUST occur in chilling, then eating order to match the
    // player_sprite_t enumeration above.
    list_t *sprites = list_init(PLAYER_SPRITE_COUNT, (free_func_t)sprite_free);
    list_add(sprites, sprite_chilling);
    list_add(sprites, sprite_eating);

    gfx_aux_t *gfx_aux = gfx_aux_init(sprites);

    body_t *body = body_init_with_gfx(INFINITY,
                                      player,
                                      (free_func_t)player_free,
                                      shapes,
                                      gfx_aux);

    return body;
}

vector_t player_get_origin(player_t *player) {
    return player->origin;
}
