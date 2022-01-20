#include "ball.h"
#include "body.h"
#include "game_world.h"
#include "player.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "shapes_geometry.h"
#include "sprite.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>

#define MIN_X 0.0
#define MIN_Y 0.0
#define MAX_X 2000
#define MAX_Y 1000
const vector_t MIN = {MIN_X, MIN_Y};
const vector_t MAX = {MAX_X, MAX_Y};
const double SPEED = 10;

/*** KEY HANDLING ***/
typedef struct key_aux {
    body_t *body;
    player_t *player;
    game_world_t *world;
    vector_t *anchor;
    vector_t *offset;
    double *angle;
} key_aux_t;

void test_on_key(char key,
                 key_event_type_t type,
                 double held_time,
                 key_aux_t *aux);

void test_on_key(char key,
                 key_event_type_t type,
                 double held_time,
                 key_aux_t *aux) {
    switch (type) {
    case KEY_PRESSED:
        switch (key) {
        case 'q':
            *(aux->angle) += M_PI / 24;
            break;
        case 'e':
            *(aux->angle) -= M_PI / 24;
            break;
        case 'h':
            aux->anchor->x -= SPEED;
            break;
        case 'j':
            aux->anchor->y -= SPEED;
            break;
        case 'k':
            aux->anchor->y += SPEED;
            break;
        case 'l':
            aux->anchor->x += SPEED;
            break;
        case '0':
            body_set_shape_main(aux->body, 0);
            break;
        case '1':
            body_set_shape_main(aux->body, 1);
            break;
        case 'w':
            player_set_state(aux->player, PLAYER_EATING);
            break;
        case 'r':
            player_set_state(aux->player, PLAYER_CHILLING);
            break;
        case 'g':
            player_activate_powerup(aux->player, aux->world);
            break;
        case 'f':
            player_inc_powerup_idx(aux->player, 1);
            break;
        }
        break;
    default:
        break;
    }
    body_set_centroid(aux->body, *(aux->anchor));
    body_set_rotation(aux->body, *(aux->angle));
}

/*** MAIN ***/
int main(int argc, char *argv[]) {
    vector_t *anchor = malloc(sizeof(vector_t));
    double *angle = malloc(sizeof(double));
    key_aux_t *key_aux = malloc(sizeof(key_aux_t));

    key_aux->anchor = anchor;
    key_aux->angle = angle;
    sdl_on_key((key_handler_t)test_on_key, key_aux);
    sdl_init(MIN, MAX);

    *anchor = (vector_t){MAX_X / 2, MAX_Y / 2};
    // vector_t offset = VEC_ZERO;// (vector_t){100, 100};
    *angle = 0;

    // char path[] = "static/hippo/hippo1.png";
    // char shape_path[] = "static/hippo/hippo_collision_shape.csv";

    scene_t *scene = scene_init();
    scene_set_dims(scene, MIN, MAX);

    player_t *player = player_init(0);
    player_t *player2 = player_init(1);
    player_set_state(player2, PLAYER_EATING);

    player_add_powerup(player, powerup_init(POWER_PLAYER_SHOOT_KILL));
    player_add_powerup(player, powerup_init(POWER_PLAYER_SHOOT_SLOW));
    player_add_powerup(player, powerup_init(POWER_PLAYER_SHOOT_SPILL));

    key_aux->player = player;

    list_t *hippos = list_init(1, NULL);
    list_add(hippos, player);
    list_add(hippos, player2);

    game_world_t *w = game_world_init(scene, 1, 6.28, SPEED, 1, *anchor);
    game_world_set_hippos(w, hippos);

    key_aux->world = w;

    // list_t *shape0 = polygon_init_from_path(shape_path);
    /* polygon_scale(shape0, 1.0/6); */
    // list_t *shape1 = make_rectangle_polygon(*anchor, 100, 100);
    // list_t *shapes = list_init(2, (free_func_t)list_free);
    // list_add(shapes, shape0);
    // list_add(shapes, shape1);

    // sprite_t *sprite = sprite_init(path, 1.0/6, offset);
    /* sprite_setup(sprite, anchor, angle);*/
    // list_t *sprites = list_init(1, (free_func_t)sprite_free);
    // list_add(sprites, sprite);

    // gfx_aux_t *gfx_aux = gfx_aux_init(sprites);

    body_t *body1 = player_get_body(player);
    body_t *body2 = player_get_body(player2);

    scene_add_body(scene, body1);
    scene_add_body(scene, body2);

    body_set_centroid(body1, *anchor);
    body_set_rotation(body1, *angle);

    key_aux->body = body1;

    double dt;
    double dt_sum = 0;
    int power_idx = 0;
    while (!sdl_is_done()) {
        dt = time_since_last_tick();
        dt_sum += dt;
        scene_tick(scene, dt);
        /*
        if (dt_sum >= 1) {
            spawn_ball(w, game_world_get_arena_center(w), vec_multiply(SPEED*50,
        E1), power_idx % POWER_NUM_TYPES); dt_sum = 0; power_idx++;
        }
        */
        sdl_clear();
        sdl_render_scene(scene);
        sdl_show();
    }

    // body_free(body);
    player_free(player);
    free(anchor);
    free(angle);
}
