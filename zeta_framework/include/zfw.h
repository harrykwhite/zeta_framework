#ifndef __ZFW_GAME_H__
#define __ZFW_GAME_H__

#include <GLFW/glfw3.h>
#include <zfw_common.h>
#include "glad/glad.h"
#include "zfw_utils.h"
#include "zfw_input_codes.h"
#include "zfw_builtin_shader_prog_srcs.h"

#define ZFW_RENDER_LAYER_LIMIT 12
#define ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT ZFW_SIZE_IN_BITS (zfw_render_layer_sprite_batch_activity_bits_t)
#define ZFW_SPRITE_BATCH_SLOT_LIMIT 4096
#define ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT 32

typedef struct
{
	zfw_vec_2d_i_t size;
	zfw_vec_2d_i_t pos;
} zfw_window_state_t;

typedef unsigned long long zfw_keys_down_bits_t;
typedef unsigned char zfw_mouse_buttons_down_bits_t;
typedef unsigned short zfw_gamepad_buttons_down_bits_t;

typedef struct
{
	zfw_keys_down_bits_t keys_down_bits;
	zfw_mouse_buttons_down_bits_t mouse_buttons_down_bits;
	zfw_gamepad_buttons_down_bits_t gamepad_buttons_down_bits;

	zfw_vec_2d_t mouse_pos;

	int gamepad_glfw_joystick_index;
	float gamepad_axis_values[ZFW_NUM_GAMEPAD_AXIS_CODES];
} zfw_input_state_t;

typedef struct
{
	int tex_count;

	GLuint *gl_ids;
	zfw_vec_2d_i_t *sizes;
} zfw_user_tex_data_t;

typedef struct
{
	int prog_count;
	GLuint *gl_ids;
} zfw_user_shader_prog_data_t;

typedef struct
{
	zfw_user_tex_data_t tex_data;
	zfw_user_shader_prog_data_t shader_prog_data;
} zfw_user_asset_data_t;

typedef struct
{
	GLuint textured_rect_prog_gl_id;
} zfw_builtin_shader_prog_data_t;

typedef unsigned char zfw_render_layer_sprite_batch_activity_bits_t;
typedef unsigned long long zfw_sprite_batch_slot_activity_bits_t;

typedef struct
{
	zfw_render_layer_sprite_batch_activity_bits_t batch_activities[ZFW_RENDER_LAYER_LIMIT];

	GLuint vert_array_gl_ids[ZFW_RENDER_LAYER_LIMIT][ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT];
	GLuint vert_buf_gl_ids[ZFW_RENDER_LAYER_LIMIT][ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT];
	GLuint elem_buf_gl_ids[ZFW_RENDER_LAYER_LIMIT][ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT];

	zfw_sprite_batch_slot_activity_bits_t slot_activity_bits[ZFW_RENDER_LAYER_LIMIT][ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT][ZFW_SPRITE_BATCH_SLOT_LIMIT / sizeof(zfw_sprite_batch_slot_activity_bits_t)];

	int user_tex_indexes[ZFW_RENDER_LAYER_LIMIT][ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT][ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT];
} zfw_sprite_batch_data_t;

typedef struct
{
	int batch_index;
	int batch_slot_index;
	int batch_tex_unit;
} zfw_sprite_batch_slot_key_t;

typedef struct
{
	zfw_vec_2d_t pos;
	float scale;
} zfw_view_state_t;

// DESCRIPTION: Game state data to be provided to the user in their defined game initialization function.
typedef struct
{
	zfw_vec_2d_i_t window_size;

	const zfw_user_asset_data_t *user_asset_data;
	const zfw_builtin_shader_prog_data_t *builtin_shader_prog_data;

	zfw_sprite_batch_data_t *view_sprite_batch_data;
	zfw_sprite_batch_data_t *screen_sprite_batch_data;
	zfw_view_state_t *view_state;
} zfw_user_game_init_func_data_t;

// DESCRIPTION: Game state data to be provided to the user in their defined game tick function.
typedef struct
{
	int *windowed;
	zfw_vec_2d_i_t window_size;

	const zfw_input_state_t *input_state;
	const zfw_input_state_t *input_state_last;

	const zfw_user_asset_data_t *user_asset_data;

	zfw_sprite_batch_data_t *view_sprite_batch_data;
	zfw_sprite_batch_data_t *screen_sprite_batch_data;
	zfw_view_state_t *view_state;
} zfw_user_game_tick_func_data_t;

// DESCRIPTION: Game state data to be provided to the user in their defined window resize function.
typedef struct
{
	zfw_vec_2d_i_t window_size;
} zfw_user_window_resize_func_data_t;

typedef void (*zfw_on_game_init_func_t)(void *, zfw_user_game_init_func_data_t *);
typedef void (*zfw_on_game_tick_func_t)(void *, zfw_user_game_tick_func_data_t *);
typedef void (*zfw_on_game_clean_func_t)(void *);
typedef void (*zfw_on_window_resize_func_t)(void *, zfw_user_window_resize_func_data_t *);

// DESCRIPTION: Key game information to be defined by the user and used when the game runs.
typedef struct
{
	zfw_vec_2d_i_t init_window_size;
	const char *init_window_title;
	zfw_bool_t window_resizable;

	zfw_on_game_init_func_t on_init_func;
	zfw_on_game_tick_func_t on_tick_func;
	zfw_on_game_clean_func_t on_clean_func;
	zfw_on_window_resize_func_t on_window_resize_func;

	void *user_ptr; // NOTE: This can be a pointer to anything, and allows for state to persist across user-defined functions without needing to use global variables.
} zfw_game_run_info_t;

zfw_bool_t zfw_run_game(const zfw_game_run_info_t *const run_info);

zfw_bool_t zfw_is_key_down(const zfw_key_code_t key_code, const zfw_input_state_t *const input_state);
zfw_bool_t zfw_is_key_pressed(const zfw_key_code_t key_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last);
zfw_bool_t zfw_is_key_released(const zfw_key_code_t key_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last);

zfw_bool_t zfw_is_mouse_button_down(const zfw_mouse_button_code_t button_code, const zfw_input_state_t *const input_state);
zfw_bool_t zfw_is_mouse_button_pressed(const zfw_mouse_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last);
zfw_bool_t zfw_is_mouse_button_released(const zfw_mouse_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last);

zfw_bool_t zfw_is_gamepad_button_down(const zfw_gamepad_button_code_t button_code, const zfw_input_state_t *const input_state);
zfw_bool_t zfw_is_gamepad_button_pressed(const zfw_gamepad_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last);
zfw_bool_t zfw_is_gamepad_button_released(const zfw_gamepad_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last);

void zfw_set_up_shader_prog(GLuint *const shader_prog_gl_id, const char *const vert_shader_src, const char *const frag_shader_src);

zfw_bool_t zfw_take_slot_from_render_layer_sprite_batch(const int layer_index, const int user_tex_index, zfw_sprite_batch_slot_key_t *const key, zfw_sprite_batch_data_t *const batch_data);
void zfw_write_to_render_layer_sprite_batch_slot(const int layer_index, const zfw_sprite_batch_slot_key_t *const key, const zfw_vec_2d_t pos, const float rot, const zfw_vec_2d_t scale, const zfw_vec_2d_t origin, const float opacity, const zfw_sprite_batch_data_t *const batch_data, const zfw_user_tex_data_t *const user_tex_data);

zfw_vec_2d_t zfw_get_view_to_screen_pos(const zfw_vec_2d_t pos, const zfw_view_state_t *const view_state);
zfw_vec_2d_t zfw_get_screen_to_view_pos(const zfw_vec_2d_t pos, const zfw_view_state_t *const view_state);

#endif
