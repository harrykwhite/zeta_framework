#ifndef __ZFW_GAME_H__
#define __ZFW_GAME_H__

#include <zfw_common_mem.h>
#include <zfw_common_math.h>
#include <zfw_common_misc.h>
#include "zfw_input.h"
#include "zfw_assets.h"
#include "zfw_rendering.h"

#define ZFW_MAIN_MEM_ARENA_SIZE ((1 << 20) * 10)

// Core game data to be provided to the user in their defined functions.
typedef struct
{
    zfw_mem_arena_t *main_mem_arena;

    zfw_bool_t *restart;

    zfw_vec_2d_i_t window_size;
    zfw_bool_t *window_fullscreen;

    const zfw_input_state_t *input_state;
    const zfw_input_state_t *input_state_last;

    const zfw_user_tex_data_t *user_tex_data;
    const zfw_user_shader_prog_data_t *user_shader_prog_data;
    const zfw_user_font_data_t *user_font_data;

    zfw_sprite_batch_group_t *sprite_batch_groups;
    zfw_char_batch_group_t *char_batch_group;
    zfw_view_state_t *view_state;
} zfw_user_func_data_t;

typedef void (*zfw_on_game_init_user_func_t)(void *const, zfw_user_func_data_t *const);
typedef void (*zfw_on_game_tick_user_func_t)(void *const, zfw_user_func_data_t *const, const int, const double);
typedef void (*zfw_on_window_resize_user_func_t)(void *const, zfw_user_func_data_t *const);

// Key game information to be defined by the user and used when the game runs.
typedef struct
{
    zfw_vec_2d_i_t init_window_size;
    const char *init_window_title;
    zfw_bool_t init_window_fullscreen;
    zfw_bool_t window_resizable;

    zfw_on_game_init_user_func_t on_init_func;
    zfw_on_game_tick_user_func_t on_tick_func;
    zfw_on_window_resize_user_func_t on_window_resize_func;

    void *user_ptr;
} zfw_user_game_run_info_t;

zfw_bool_t zfw_run_game(const zfw_user_game_run_info_t *const user_run_info);

#endif
