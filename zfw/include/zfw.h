#ifndef __ZFW_H__
#define __ZFW_H__

#include <stdio.h>
#include <glad/glad.h>
#include <zfw_common.h>

////// Memory Arena Sizes //////
#define ZFW_MEM_ARENA_SIZE ((1 << 20) * 100)

////// Built-in Shader Sources //////
#define ZFW_BUILTIN_TEXTURED_RECT_VERT_SHADER_SRC \
    "#version 430 core\n" \
    "\n" \
    "layout (location = 0) in vec2 a_vert;\n" \
    "layout (location = 1) in vec2 a_pos;\n" \
    "layout (location = 2) in vec2 a_size;\n" \
    "layout (location = 3) in float a_rot;\n" \
    "layout (location = 4) in float a_tex_index;\n" \
    "layout (location = 5) in vec2 a_tex_coord;\n" \
    "layout (location = 6) in vec4 a_blend;\n" \
    "\n" \
    "out flat int v_tex_index;\n" \
    "out vec2 v_tex_coord;\n" \
    "out vec4 v_blend;\n" \
    "\n" \
    "uniform mat4 u_view;\n" \
    "uniform mat4 u_proj;\n" \
    "\n" \
    "void main()\n" \
    "{\n" \
    "    float rot_cos = cos(a_rot);\n" \
    "    float rot_sin = -sin(a_rot);\n" \
    "\n" \
    "    mat4 model = mat4(\n" \
    "        vec4(a_size.x * rot_cos, a_size.x * rot_sin, 0.0f, 0.0f),\n" \
    "        vec4(a_size.y * -rot_sin, a_size.y * rot_cos, 0.0f, 0.0f),\n" \
    "        vec4(0.0f, 0.0f, 1.0f, 0.0f),\n" \
    "        vec4(a_pos.x, a_pos.y, 0.0f, 1.0f)\n" \
    "    );\n" \
    "\n" \
    "    gl_Position = u_proj * u_view * model * vec4(a_vert, 0.0f, 1.0f);\n" \
    "\n" \
    "    v_tex_index = int(a_tex_index);\n" \
    "    v_tex_coord = a_tex_coord;\n" \
    "    v_blend = a_blend;\n" \
    "}\n"

#define ZFW_BUILTIN_TEXTURED_RECT_FRAG_SHADER_SRC \
    "#version 430 core\n" \
    "\n" \
    "in flat int v_tex_index;\n" \
    "in vec2 v_tex_coord;\n" \
    "in vec4 v_blend;\n" \
    "\n" \
    "out vec4 o_frag_color;\n" \
    "\n" \
    "uniform sampler2D u_textures[32];\n" \
    "\n" \
    "void main()\n" \
    "{\n" \
    "    vec4 tex_color = texture(u_textures[v_tex_index], v_tex_coord);\n" \
    "    o_frag_color = tex_color * v_blend;\n" \
    "}\n"

#define ZFW_BUILTIN_TEXTURED_RECT_SHADER_PROG_VERT_COUNT 14

#define ZFW_BUILTIN_CHAR_RECT_VERT_SHADER_SRC \
    "#version 430 core\n" \
    "\n" \
    "layout (location = 0) in vec2 a_vert;\n" \
    "layout (location = 1) in vec2 a_tex_coord;\n" \
    "\n" \
    "out vec2 v_tex_coord;\n" \
    "\n" \
    "uniform vec2 u_pos;\n" \
    "uniform vec2 u_scale;\n" \
    "uniform mat4 u_proj;\n" \
    "\n" \
    "void main()\n" \
    "{\n" \
    "    mat4 model = mat4(\n" \
    "        vec4(u_scale.x, 0.0f, 0.0f, 0.0f),\n" \
    "        vec4(0.0f, u_scale.y, 0.0f, 0.0f),\n" \
    "        vec4(0.0f, 0.0f, 1.0f, 0.0f),\n" \
    "        vec4(u_pos.x, u_pos.y, 0.0f, 1.0f)\n" \
    "    );\n" \
    "    gl_Position = u_proj * model * vec4(a_vert, 0.0f, 1.0f);\n" \
    "\n" \
    "    v_tex_coord = a_tex_coord;\n" \
    "}\n"

#define ZFW_BUILTIN_CHAR_RECT_FRAG_SHADER_SRC \
    "#version 430 core\n" \
    "\n" \
    "in vec2 v_tex_coord;\n" \
    "\n" \
    "out vec4 o_frag_color;\n" \
    "\n" \
    "uniform vec4 u_blend;\n" \
    "uniform sampler2D u_tex;\n" \
    "\n" \
    "void main()\n" \
    "{\n" \
    "    vec4 tex_color = texture(u_tex, v_tex_coord);\n" \
    "    o_frag_color = tex_color * u_blend;\n" \
    "}\n"

#define ZFW_BUILTIN_CHAR_RECT_SHADER_PROG_VERT_COUNT 4

////// Rendering Limits //////
// These must all be powers of 2!
#define ZFW_RENDER_LAYER_LIMIT (1 << 5)

#define ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT ZFW_SIZE_IN_BITS(zfw_render_layer_sprite_batch_activity_bits_t)
#define ZFW_SPRITE_BATCH_SLOT_LIMIT (1 << 13)
#define ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT (1 << 5)

#define ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT ZFW_SIZE_IN_BITS(zfw_render_layer_char_batch_bits_t)
#define ZFW_CHAR_BATCH_SLOT_LIMIT (1 << 6)

/////// Input Code Enums //////
typedef enum
{
    ZFW_KEY_CODE__SPACE,

    ZFW_KEY_CODE__0,
    ZFW_KEY_CODE__1,
    ZFW_KEY_CODE__2,
    ZFW_KEY_CODE__3,
    ZFW_KEY_CODE__4,
    ZFW_KEY_CODE__5,
    ZFW_KEY_CODE__6,
    ZFW_KEY_CODE__7,
    ZFW_KEY_CODE__8,
    ZFW_KEY_CODE__9,

    ZFW_KEY_CODE__A,
    ZFW_KEY_CODE__B,
    ZFW_KEY_CODE__C,
    ZFW_KEY_CODE__D,
    ZFW_KEY_CODE__E,
    ZFW_KEY_CODE__F,
    ZFW_KEY_CODE__G,
    ZFW_KEY_CODE__H,
    ZFW_KEY_CODE__I,
    ZFW_KEY_CODE__J,
    ZFW_KEY_CODE__K,
    ZFW_KEY_CODE__L,
    ZFW_KEY_CODE__M,
    ZFW_KEY_CODE__N,
    ZFW_KEY_CODE__O,
    ZFW_KEY_CODE__P,
    ZFW_KEY_CODE__Q,
    ZFW_KEY_CODE__R,
    ZFW_KEY_CODE__S,
    ZFW_KEY_CODE__T,
    ZFW_KEY_CODE__U,
    ZFW_KEY_CODE__V,
    ZFW_KEY_CODE__W,
    ZFW_KEY_CODE__X,
    ZFW_KEY_CODE__Y,
    ZFW_KEY_CODE__Z,

    ZFW_KEY_CODE__ESCAPE,
    ZFW_KEY_CODE__ENTER,
    ZFW_KEY_CODE__TAB,

    ZFW_KEY_CODE__RIGHT,
    ZFW_KEY_CODE__LEFT,
    ZFW_KEY_CODE__DOWN,
    ZFW_KEY_CODE__UP,

    ZFW_KEY_CODE__F1,
    ZFW_KEY_CODE__F2,
    ZFW_KEY_CODE__F3,
    ZFW_KEY_CODE__F4,
    ZFW_KEY_CODE__F5,
    ZFW_KEY_CODE__F6,
    ZFW_KEY_CODE__F7,
    ZFW_KEY_CODE__F8,
    ZFW_KEY_CODE__F9,
    ZFW_KEY_CODE__F10,
    ZFW_KEY_CODE__F11,
    ZFW_KEY_CODE__F12,

    ZFW_KEY_CODE__LEFT_SHIFT,
    ZFW_KEY_CODE__LEFT_CONTROL,
    ZFW_KEY_CODE__LEFT_ALT,

    ZFW_KEY_CODE_COUNT
} zfw_key_code_t;

typedef enum
{
    ZFW_MOUSE_BUTTON_CODE__LEFT,
    ZFW_MOUSE_BUTTON_CODE__RIGHT,
    ZFW_MOUSE_BUTTON_CODE__MIDDLE,

    ZFW_MOUSE_BUTTON_CODE_COUNT
} zfw_mouse_button_code_t;

typedef enum
{
    ZFW_GAMEPAD_BUTTON_CODE__A,
    ZFW_GAMEPAD_BUTTON_CODE__B,
    ZFW_GAMEPAD_BUTTON_CODE__X,
    ZFW_GAMEPAD_BUTTON_CODE__Y,

    ZFW_GAMEPAD_BUTTON_CODE__LEFT_BUMPER,
    ZFW_GAMEPAD_BUTTON_CODE__RIGHT_BUMPER,

    ZFW_GAMEPAD_BUTTON_CODE__BACK,
    ZFW_GAMEPAD_BUTTON_CODE__START,
    ZFW_GAMEPAD_BUTTON_CODE__GUIDE,

    ZFW_GAMEPAD_BUTTON_CODE__LEFT_THUMB,
    ZFW_GAMEPAD_BUTTON_CODE__RIGHT_THUMB,

    ZFW_GAMEPAD_BUTTON_CODE__DPAD_UP,
    ZFW_GAMEPAD_BUTTON_CODE__DPAD_RIGHT,
    ZFW_GAMEPAD_BUTTON_CODE__DPAD_DOWN,
    ZFW_GAMEPAD_BUTTON_CODE__DPAD_LEFT,

    ZFW_GAMEPAD_BUTTON_CODE_COUNT
} zfw_gamepad_button_code_t;

typedef enum
{
    ZFW_GAMEPAD_AXIS_CODE__LEFT_X,
    ZFW_GAMEPAD_AXIS_CODE__LEFT_Y,

    ZFW_GAMEPAD_AXIS_CODE__RIGHT_X,
    ZFW_GAMEPAD_AXIS_CODE__RIGHT_Y,

    ZFW_GAMEPAD_AXIS_CODE__LEFT_TRIGGER,
    ZFW_GAMEPAD_AXIS_CODE__RIGHT_TRIGGER,

    ZFW_GAMEPAD_AXIS_CODE_COUNT
} zfw_gamepad_axis_code_t;

////// Rendering Enums //////
typedef enum
{
    ZFW_SPRITE_BATCH_GROUP_ID__VIEW,
    ZFW_SPRITE_BATCH_GROUP_ID__SCREEN,

    ZFW_SPRITE_BATCH_GROUP_ID_COUNT // This must be a power of 2!
} zfw_sprite_batch_group_id_t;

typedef enum
{
    ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__ACTIVE,
    ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_GROUP_INDEX,
    ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__LAYER_INDEX,
    ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_INDEX,
    ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__SLOT_INDEX,
    ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__TEX_UNIT_INDEX,

    ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID_COUNT
} zfw_sprite_batch_slot_key_elem_id_t;

typedef enum
{
    ZFW_FONT_HOR_ALIGN__LEFT,
    ZFW_FONT_HOR_ALIGN__CENTER,
    ZFW_FONT_HOR_ALIGN__RIGHT
} zfw_font_hor_align_t;

typedef enum
{
    ZFW_FONT_VERT_ALIGN__TOP,
    ZFW_FONT_VERT_ALIGN__MIDDLE,
    ZFW_FONT_VERT_ALIGN__BOTTOM
} zfw_font_vert_align_t;

////// Input Bits //////
typedef unsigned long long zfw_keys_down_bits_t;
typedef unsigned char zfw_mouse_buttons_down_bits_t;
typedef unsigned short zfw_gamepad_buttons_down_bits_t;

////// Rendering Type Definitions //////
typedef unsigned char zfw_render_layer_sprite_batch_activity_bits_t;
typedef unsigned int zfw_sprite_batch_slot_key_t;

typedef unsigned long long zfw_render_layer_char_batch_bits_t;
typedef unsigned short zfw_char_batch_key_t;

////// Utility Structs //////
typedef struct
{
    unsigned char *bytes;
    int byte_count;
} zfw_bitset_t;

typedef struct
{
    float r, g, b, a;
} zfw_color_t;

////// Window and Input Structs //////
typedef struct
{
    zfw_vec_2d_i_t size;
    zfw_vec_2d_i_t pos;
    zfw_bool_t fullscreen;
} zfw_window_state_t;

typedef struct
{
    zfw_keys_down_bits_t keys_down_bits;
    zfw_mouse_buttons_down_bits_t mouse_buttons_down_bits;
    zfw_gamepad_buttons_down_bits_t gamepad_buttons_down_bits;

    zfw_vec_2d_t mouse_pos;
    int mouse_scroll;

    int gamepad_glfw_joystick_index;
    float gamepad_axis_values[ZFW_GAMEPAD_AXIS_CODE_COUNT];
} zfw_input_state_t;

////// Asset Structs //////
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
    int font_count;

    int *line_heights;

    font_char_hor_offs_t *chars_hor_offsets;
    font_char_vert_offs_t *chars_vert_offsets;

    font_char_hor_advance_t *chars_hor_advances;

    font_char_src_rect_t *chars_src_rects;

    font_char_kerning_t *chars_kernings;

    zfw_vec_2d_i_t *tex_sizes;
    GLuint *tex_glids;
} zfw_user_font_data_t;

typedef struct
{
    zfw_user_tex_data_t tex_data;
    zfw_user_shader_prog_data_t shader_prog_data;
    zfw_user_font_data_t font_data;
} zfw_user_asset_data_t;

typedef struct
{
    GLuint textured_rect_prog_gl_id;
    GLuint char_rect_prog_gl_id;
} zfw_builtin_shader_prog_data_t;

////// Rendering Structs //////
typedef struct
{
    int user_tex_index;
    int count; // Represents the number of slots in the batch that are mapped to this texture unit.
} zfw_sprite_batch_tex_unit_t;

typedef struct
{
    zfw_render_layer_sprite_batch_activity_bits_t batch_activity_bits[ZFW_RENDER_LAYER_LIMIT];

    GLuint vert_array_gl_ids[ZFW_RENDER_LAYER_LIMIT][ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT];
    GLuint vert_buf_gl_ids[ZFW_RENDER_LAYER_LIMIT][ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT];
    GLuint elem_buf_gl_ids[ZFW_RENDER_LAYER_LIMIT][ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT];

    zfw_sprite_batch_tex_unit_t tex_units[ZFW_RENDER_LAYER_LIMIT][ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT][ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT];

    zfw_bitset_t slot_activity_bitset;
} zfw_sprite_batch_group_t;

typedef struct
{
    zfw_bool_t active;
    int batch_group_index;
    int layer_index;
    int batch_index;
    int slot_index;
    int tex_unit_index;
} zfw_sprite_batch_slot_key_elems_t;

typedef struct
{
    zfw_render_layer_char_batch_bits_t batch_init_bits[ZFW_RENDER_LAYER_LIMIT]; // Each bit represents whether the corresponding batch has been initialized.
    zfw_render_layer_char_batch_bits_t batch_activity_bits[ZFW_RENDER_LAYER_LIMIT];

    GLuint vert_array_gl_ids[ZFW_RENDER_LAYER_LIMIT][ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT];
    GLuint vert_buf_gl_ids[ZFW_RENDER_LAYER_LIMIT][ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT];
    GLuint elem_buf_gl_ids[ZFW_RENDER_LAYER_LIMIT][ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT];

    int user_font_indexes[ZFW_RENDER_LAYER_LIMIT][ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT];
    zfw_vec_2d_t positions[ZFW_RENDER_LAYER_LIMIT][ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT];
    zfw_vec_2d_t scales[ZFW_RENDER_LAYER_LIMIT][ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT];
    zfw_color_t blends[ZFW_RENDER_LAYER_LIMIT][ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT];
} zfw_char_batch_group_t;

typedef struct
{
    zfw_bool_t active;
    int layer_index;
    int batch_index;
} zfw_char_batch_key_elems_t;

typedef struct
{
    zfw_vec_2d_t pos;
    float scale;
} zfw_view_state_t;

////// Game Structs and Function Pointer Type Definitions //////
// Game state data to be provided to the user in their defined game functions.
typedef struct
{
    zfw_mem_arena_t *mem_arena;
    
    zfw_bool_t *restart;

    zfw_vec_2d_i_t window_size;
    zfw_bool_t *window_fullscreen;

    const zfw_input_state_t *input_state;
    const zfw_input_state_t *input_state_last;

    const zfw_user_asset_data_t *user_asset_data;
     
    zfw_sprite_batch_group_t *sprite_batch_groups;
    zfw_char_batch_group_t *char_batch_group;
    zfw_view_state_t *view_state;
} zfw_user_game_func_data_t;

typedef void (*zfw_on_game_init_func_t)(void *, zfw_user_game_func_data_t *);
typedef void (*zfw_on_game_tick_func_t)(void *, zfw_user_game_func_data_t *);
typedef void (*zfw_on_window_resize_func_t)(void *, zfw_user_game_func_data_t *);

// Key game information to be defined by the user and used when the game runs.
typedef struct
{
    zfw_vec_2d_i_t init_window_size;
    const char *init_window_title;
    zfw_bool_t init_window_fullscreen;
    zfw_bool_t window_resizable;

    zfw_on_game_init_func_t on_init_func;
    zfw_on_game_tick_func_t on_tick_func;
    zfw_on_window_resize_func_t on_window_resize_func;

    void *user_ptr;
} zfw_user_game_run_info_t;

////// Color Constants //////
extern const zfw_color_t zfw_k_color_white;
extern const zfw_color_t zfw_k_color_black;
extern const zfw_color_t zfw_k_color_red;
extern const zfw_color_t zfw_k_color_green;
extern const zfw_color_t zfw_k_color_blue;
extern const zfw_color_t zfw_k_color_yellow;

////// Utility Functions //////
float zfw_get_clamped_float(const float n, const float min, const float max);
int zfw_get_int_digit_count(const int n);

zfw_bool_t zfw_init_bitset(zfw_bitset_t *const bitset, const int byte_count);
void zfw_toggle_bitset_bit(zfw_bitset_t *const bitset, const int bit_index, const int bit_active);
void zfw_clear_bitset(zfw_bitset_t *const bitset);
void zfw_clean_bitset(zfw_bitset_t *const bitset);
int zfw_get_first_inactive_bitset_bit_index(const zfw_bitset_t *const bitset);
int zfw_get_first_inactive_bitset_bit_index_in_range(const zfw_bitset_t *const bitset, const int begin_bit_index, const int end_bit_index);
zfw_bool_t zfw_is_bitset_bit_active(const zfw_bitset_t *const bitset, const int bit_index);
zfw_bool_t zfw_is_bitset_fully_active(const zfw_bitset_t *const bitset);
zfw_bool_t zfw_is_bitset_clear(const zfw_bitset_t *const bitset);

void zfw_init_color(zfw_color_t *const color, const float r, const float g, const float b, const float a);

float zfw_gen_rand_num(); // Generates a random float number between 0 and 1 inclusive.
float zfw_gen_rand_num_in_range(const float min, const float max);

////// Game Functions //////
zfw_bool_t zfw_run_game(const zfw_user_game_run_info_t *const user_run_info);

////// Input Functions //////
void zfw_reset_gamepad_state(zfw_input_state_t *const input_state);

inline zfw_bool_t zfw_is_key_down(const zfw_key_code_t key_code, const zfw_input_state_t *const input_state)
{
    return (input_state->keys_down_bits & ((zfw_keys_down_bits_t)1 << key_code)) != 0;
}

inline zfw_bool_t zfw_is_key_pressed(const zfw_key_code_t key_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return zfw_is_key_down(key_code, input_state) && !zfw_is_key_down(key_code, input_state_last);
}

inline zfw_bool_t zfw_is_key_released(const zfw_key_code_t key_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return !zfw_is_key_down(key_code, input_state) && zfw_is_key_down(key_code, input_state_last);
}

inline zfw_bool_t zfw_is_mouse_button_down(const zfw_mouse_button_code_t button_code, const zfw_input_state_t *const input_state)
{
    return (input_state->mouse_buttons_down_bits & ((zfw_mouse_buttons_down_bits_t)1 << button_code)) != 0;
}

inline zfw_bool_t zfw_is_mouse_button_pressed(const zfw_mouse_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return zfw_is_mouse_button_down(button_code, input_state) && !zfw_is_mouse_button_down(button_code, input_state_last);
}

inline zfw_bool_t zfw_is_mouse_button_released(const zfw_mouse_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return !zfw_is_mouse_button_down(button_code, input_state) && zfw_is_mouse_button_down(button_code, input_state_last);
}

inline zfw_bool_t zfw_is_gamepad_button_down(const zfw_gamepad_button_code_t button_code, const zfw_input_state_t *const input_state)
{
    return (input_state->gamepad_buttons_down_bits & ((zfw_gamepad_buttons_down_bits_t)1 << button_code)) != 0;
}

inline zfw_bool_t zfw_is_gamepad_button_pressed(const zfw_gamepad_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return zfw_is_gamepad_button_down(button_code, input_state) && !zfw_is_gamepad_button_down(button_code, input_state_last);
}

inline zfw_bool_t zfw_is_gamepad_button_released(const zfw_gamepad_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return !zfw_is_gamepad_button_down(button_code, input_state) && zfw_is_gamepad_button_down(button_code, input_state_last);
}

////// Asset Functions //////
zfw_bool_t zfw_retrieve_user_asset_data_from_assets_file(zfw_user_asset_data_t *const user_asset_data, FILE *const assets_file_fs, zfw_mem_arena_t *const mem_arena);
void zfw_gen_shader_prog(GLuint *const shader_prog_gl_id, const char *const vert_shader_src, const char *const frag_shader_src);

////// Rendering Functions //////
zfw_sprite_batch_slot_key_t zfw_take_slot_from_render_layer_sprite_batch(const zfw_sprite_batch_group_id_t batch_group_id, const int layer_index, const int user_tex_index, zfw_sprite_batch_group_t *const batch_groups, zfw_mem_arena_t *const mem_arena);
void zfw_take_multiple_slots_from_render_layer_sprite_batch(zfw_sprite_batch_slot_key_t *const slot_keys, const int slot_key_count, const zfw_sprite_batch_group_id_t batch_group_id, const int layer_index, const int user_tex_index, zfw_sprite_batch_group_t *const batch_groups, zfw_mem_arena_t *const mem_arena); // This function will not set all the slot keys to be inactive for you, should not all slots be taken.
zfw_bool_t zfw_write_to_render_layer_sprite_batch_slot(const zfw_sprite_batch_slot_key_t slot_key, const zfw_vec_2d_t pos, const float rot, const zfw_vec_2d_t scale, const zfw_vec_2d_t origin, const zfw_rect_t *const src_rect, const zfw_color_t *const blend, const zfw_sprite_batch_group_t *const batch_groups, const zfw_user_tex_data_t *const user_tex_data);
zfw_bool_t zfw_clear_render_layer_sprite_batch_slot(const zfw_sprite_batch_slot_key_t slot_key, const zfw_sprite_batch_group_t *const batch_groups);
zfw_bool_t zfw_free_render_layer_sprite_batch_slot(const zfw_sprite_batch_slot_key_t slot_key, zfw_sprite_batch_group_t *const batch_groups);
zfw_sprite_batch_slot_key_t zfw_create_sprite_batch_slot_key(const zfw_sprite_batch_slot_key_elems_t *const slot_key_elems);
void zfw_init_sprite_batch_slot_key_elems(const zfw_sprite_batch_slot_key_t slot_key, zfw_sprite_batch_slot_key_elems_t *const slot_key_elems);

zfw_char_batch_key_t zfw_take_render_layer_char_batch(const int layer_index, zfw_char_batch_group_t *const batch_group, zfw_mem_arena_t *const mem_arena);
zfw_bool_t zfw_write_to_render_layer_char_batch(const zfw_char_batch_key_t key, const char *const text, const zfw_font_hor_align_t hor_align, const zfw_font_vert_align_t vert_align, zfw_char_batch_group_t *const batch_group, const zfw_user_font_data_t *const user_font_data); // IDEA: Have an alternative, faster function to be used when no alignment is needed.
zfw_bool_t zfw_set_render_layer_char_batch_user_font_index(const zfw_char_batch_key_t key, const int user_font_index, zfw_char_batch_group_t *const batch_group);
zfw_bool_t zfw_set_render_layer_char_batch_pos(const zfw_char_batch_key_t key, const zfw_vec_2d_t pos, zfw_char_batch_group_t *const batch_group);
zfw_bool_t zfw_set_render_layer_char_batch_scale(const zfw_char_batch_key_t key, const zfw_vec_2d_t scale, zfw_char_batch_group_t *const batch_group);
zfw_bool_t zfw_set_render_layer_char_batch_blend(const zfw_char_batch_key_t key, const zfw_color_t *const blend, zfw_char_batch_group_t *const batch_group);
zfw_bool_t zfw_clear_render_layer_char_batch(const zfw_char_batch_key_t key, zfw_char_batch_group_t *const batch_group);
zfw_bool_t zfw_free_render_layer_char_batch(const zfw_char_batch_key_t key, zfw_char_batch_group_t *const batch_group);
zfw_char_batch_key_t zfw_create_char_batch_key(const zfw_char_batch_key_elems_t *const key_elems);
void zfw_init_char_batch_key_elems(const zfw_sprite_batch_slot_key_t key, zfw_char_batch_key_elems_t *const key_elems);

void zfw_reset_view_state(zfw_view_state_t *const view_state);
zfw_vec_2d_t zfw_get_view_to_screen_pos(const zfw_vec_2d_t pos, const zfw_view_state_t *const view_state);
zfw_vec_2d_t zfw_get_screen_to_view_pos(const zfw_vec_2d_t pos, const zfw_view_state_t *const view_state);
/////////////////////////////////

#endif
