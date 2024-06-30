// NOTE: Consider breaking this up into more smaller header files.

#ifndef __ZFW_GAME_H__
#define __ZFW_GAME_H__

#include <string.h>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <zfw_common.h>

#include "zfw_utils.h"
#include "zfw_input_codes.h"
#include "zfw_builtin_shader_prog_srcs.h"

#define ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT ZFW_SIZE_IN_BITS(zfw_sprite_batch_activity_bits_t)
#define ZFW_RENDER_LAYER_SPRITE_BATCH_SLOT_LIMIT 8192
#define ZFW_RENDER_LAYER_SPRITE_BATCH_TEX_UNIT_LIMIT 32

typedef struct
{
	zfw_vec_i_t size;
	zfw_vec_i_t pos;
} zfw_window_state_t;

typedef unsigned long long zfw_keys_down_bits_t;
typedef unsigned char zfw_mouse_buttons_down_bits_t;
typedef unsigned short zfw_gamepad_buttons_down_bits_t;

typedef struct
{
	zfw_keys_down_bits_t keys_down_bits;
	zfw_mouse_buttons_down_bits_t mouse_buttons_down_bits;
	zfw_gamepad_buttons_down_bits_t gamepad_buttons_down_bits;

	zfw_vec_t mouse_pos;

	int gamepad_glfw_joystick_index;
	float gamepad_axis_values[ZFW_NUM_GAMEPAD_AXIS_CODES];
} zfw_input_state_t;

// DESCRIPTION: Window and input data intended for access through a user pointer in GLFW callback functions.
typedef struct
{
	zfw_window_state_t window_state;
	zfw_input_state_t input_state;
} zfw_glfw_window_user_data_t;

typedef struct
{
	int tex_count;

	GLuint *gl_ids;
	zfw_vec_i_t *sizes;
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
	GLuint default_render_layer_prog_gl_id;
} zfw_builtin_shader_prog_data_t;

typedef unsigned char zfw_sprite_batch_activity_bits_t;

typedef struct zfw_render_layer zfw_render_layer_t;

struct zfw_render_layer
{
	GLuint shader_prog_gl_id;

	zfw_render_layer_t *parent;

	zfw_render_layer_t **children;
	int child_count;

	GLuint framebuffer_gl_id;
	GLuint framebuffer_tex_gl_id;
	GLuint framebuffer_vert_array_gl_id;
	GLuint framebuffer_vert_buf_gl_id;
	GLuint framebuffer_elem_buf_gl_id;

	zfw_sprite_batch_activity_bits_t sprite_batch_activity_bits;
	GLuint sprite_batch_vert_array_gl_ids[ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT];
	GLuint sprite_batch_vert_buf_gl_ids[ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT];
	GLuint sprite_batch_elem_buf_gl_ids[ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT];
	int sprite_batch_slot_counts[ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT];
	int sprite_batch_slot_count_default;
	zfw_bitset_t sprite_batch_slot_activity_bitsets[ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT];
	GLuint sprite_batch_user_tex_indexes[ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT][ZFW_RENDER_LAYER_SPRITE_BATCH_TEX_UNIT_LIMIT];
};

typedef struct
{
	int batch_index;
	int batch_slot_index;
	int batch_tex_unit;
} zfw_sprite_batch_slot_key_t;

typedef struct
{
	zfw_vec_t pos;
	float scale;
} zfw_view_state_t;

// DESCRIPTION: Game state data to be provided to the user in their defined game initialization function.
typedef struct
{
	zfw_vec_i_t window_size;

	const zfw_user_asset_data_t *user_asset_data;
	const zfw_builtin_shader_prog_data_t *builtin_shader_prog_data;

	zfw_render_layer_t *root_view_render_layer;
	zfw_render_layer_t *root_noview_render_layer;

	zfw_view_state_t *view_state;
} zfw_user_game_init_func_data_t;

// DESCRIPTION: Game state data to be provided to the user in their defined game tick function.
typedef struct
{
	int *windowed;
	zfw_vec_i_t window_size;

	const zfw_input_state_t *input_state;
	const zfw_input_state_t *input_state_last;

	const zfw_user_asset_data_t *user_asset_data;

	zfw_render_layer_t *root_view_render_layer;
	zfw_render_layer_t *root_noview_render_layer;

	zfw_view_state_t *view_state;
} zfw_user_game_tick_func_data_t;

// DESCRIPTION: Game state data to be provided to the user in their defined window resize function.
typedef struct
{
	zfw_vec_i_t window_size;

	const zfw_user_asset_data_t *user_asset_data;

	zfw_render_layer_t *root_view_render_layer;
	zfw_render_layer_t *root_noview_render_layer;
} zfw_user_window_resize_func_data_t;

typedef void (*zfw_on_game_init_func_t)(void *, zfw_user_game_init_func_data_t *);
typedef void (*zfw_on_game_tick_func_t)(void *, zfw_user_game_tick_func_data_t *);
typedef void (*zfw_on_game_clean_func_t)(void *);
typedef void (*zfw_on_window_resize_func_t)(void *, zfw_user_window_resize_func_data_t *);

// DESCRIPTION: Key game information to be defined by the user and used when the game runs.
typedef struct
{
	zfw_vec_i_t init_window_size;
	const char *init_window_title;
	zfw_bool_t window_resizable;

	zfw_on_game_init_func_t on_init_func;
	zfw_on_game_tick_func_t on_tick_func;
	zfw_on_game_clean_func_t on_clean_func;
	zfw_on_window_resize_func_t on_window_resize_func;

	void *user_ptr; // NOTE: This can be a pointer to anything, and allows for state to persist across user-defined functions without needing to use global variables.
} zfw_game_run_info_t;

// DESCRIPTION: Pointers to data to be cleaned up when the game ends.
typedef struct
{
	GLFWwindow *glfw_window;

	zfw_user_asset_data_t *user_asset_data;
	zfw_builtin_shader_prog_data_t *builtin_shader_prog_data;

	zfw_render_layer_t *root_view_render_layer;
	zfw_render_layer_t *root_noview_render_layer;
} zfw_game_cleanup_info_t;

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

void zfw_init_render_layer(zfw_render_layer_t *const layer, const GLuint shader_prog_gl_id, const float *const init_gl_verts, const int gl_vert_count, zfw_render_layer_t *const parent, int sprite_batch_slot_count_default, const zfw_vec_i_t window_size);

zfw_bool_t zfw_init_and_activate_render_layer_sprite_batch(zfw_render_layer_t *const layer, const int index, const int slot_count);

// DESCRIPTION: Takes an available slot in any sprite batch in the given render layer. If one is not found, and there is still room in the layer for more batches, a new batch will be initialized and the whole operation will be performed again.
zfw_bool_t zfw_take_slot_from_render_layer_sprite_batch(zfw_render_layer_t *const layer, const int user_tex_index, zfw_sprite_batch_slot_key_t *const key);

void zfw_write_to_render_layer_sprite_batch_slot(zfw_render_layer_t *const layer, const zfw_sprite_batch_slot_key_t *const key, const zfw_vec_t pos, const float rot, const zfw_vec_t scale, const zfw_vec_t origin, const float depth, const float opacity, const zfw_user_tex_data_t *const user_tex_data);

#endif
