#ifndef __ZFW_RENDERING_H__
#define __ZFW_RENDERING_H__

#include <math.h>
#include <glad/glad.h>
#include <zfw_common.h>
#include "zfw_assets.h"
#include "zfw_utils.h"

typedef unsigned char zfw_render_layer_sprite_batch_activity_bits_t;
typedef unsigned long long zfw_render_layer_char_batch_bits_t;

#define ZFW_RENDER_LAYER_LIMIT 32
#define ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT ZFW_SIZE_IN_BITS(zfw_render_layer_sprite_batch_activity_bits_t)
#define ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT ZFW_SIZE_IN_BITS(zfw_render_layer_char_batch_bits_t)

#define ZFW_SPRITE_BATCH_SLOT_LIMIT 8192
#define ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT 32

#define ZFW_CHAR_BATCH_SLOT_LIMIT 64

typedef enum
{
    ZFW_SPRITE_BATCH_GROUP_ID__VIEW,
    ZFW_SPRITE_BATCH_GROUP_ID__SCREEN,

    ZFW_SPRITE_BATCH_GROUP_COUNT
} zfw_sprite_batch_group_id_t;

typedef enum
{
    ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_GROUP_INDEX,
    ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__LAYER_INDEX,
    ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_INDEX,
    ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__SLOT_INDEX,
    ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__TEX_UNIT_INDEX,

    ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_COUNT
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

typedef unsigned int zfw_sprite_batch_slot_key_t;
typedef unsigned short zfw_char_batch_key_t;

typedef struct
{
    float r, g, b, a;
} zfw_color_t;

typedef struct
{
    int user_tex_index;
    int count; // Represents the number of batch slots that are mapped to this texture unit.
} zfw_sprite_batch_tex_unit_t;

typedef struct
{
    zfw_render_layer_sprite_batch_activity_bits_t batch_activity_bits[ZFW_RENDER_LAYER_LIMIT];

    GLuint *vert_array_gl_ids;
    GLuint *vert_buf_gl_ids;
    GLuint *elem_buf_gl_ids;

    zfw_sprite_batch_tex_unit_t *tex_units;

    zfw_bitset_t slot_activity;
} zfw_sprite_batch_group_t;

typedef struct
{
    zfw_render_layer_char_batch_bits_t batch_init_bits[ZFW_RENDER_LAYER_LIMIT]; // Each bit represents whether the corresponding batch has been initialized.
    zfw_render_layer_char_batch_bits_t batch_activity_bits[ZFW_RENDER_LAYER_LIMIT];

    GLuint *vert_array_gl_ids;
    GLuint *vert_buf_gl_ids;
    GLuint *elem_buf_gl_ids;

    int *user_font_indexes;
    zfw_vec_2d_t *positions;
    zfw_vec_2d_t *scales;
    zfw_color_t *blends;
} zfw_char_batch_group_t;

typedef struct
{
    zfw_vec_2d_t pos;
    float scale;
} zfw_view_state_t;

extern const zfw_color_t zfw_k_color_white;
extern const zfw_color_t zfw_k_color_black;
extern const zfw_color_t zfw_k_color_red;
extern const zfw_color_t zfw_k_color_green;
extern const zfw_color_t zfw_k_color_blue;

zfw_bool_t zfw_init_sprite_batch_group(zfw_sprite_batch_group_t *const batch_group, zfw_mem_arena_t *const main_mem_arena);
void zfw_clean_sprite_batch_group(zfw_sprite_batch_group_t *const batch_group);
void zfw_set_sprite_batch_group_defaults(zfw_sprite_batch_group_t *const batch_group);

zfw_sprite_batch_slot_key_t zfw_take_render_layer_sprite_batch_slot(const zfw_sprite_batch_group_id_t batch_group_id, const int layer_index, const int user_tex_index, zfw_sprite_batch_group_t batch_groups[ZFW_SPRITE_BATCH_GROUP_COUNT], zfw_mem_arena_t *const main_mem_arena);
void zfw_take_multiple_render_layer_sprite_batch_slots(zfw_sprite_batch_slot_key_t *const slot_keys, const int slot_key_count, const zfw_sprite_batch_group_id_t batch_group_id, const int layer_index, const int user_tex_index, zfw_sprite_batch_group_t batch_groups[ZFW_SPRITE_BATCH_GROUP_COUNT], zfw_mem_arena_t *const main_mem_arena);
zfw_bool_t zfw_write_to_render_layer_sprite_batch_slot(const zfw_sprite_batch_slot_key_t slot_key, const zfw_vec_2d_t pos, const float rot, const zfw_vec_2d_t scale, const zfw_vec_2d_t origin, const zfw_rect_t *const src_rect, const zfw_color_t *const blend, const zfw_sprite_batch_group_t batch_groups[ZFW_SPRITE_BATCH_GROUP_COUNT], const zfw_user_tex_data_t *const user_tex_data);
zfw_bool_t zfw_clear_render_layer_sprite_batch_slot(const zfw_sprite_batch_slot_key_t slot_key, const zfw_sprite_batch_group_t batch_groups[ZFW_SPRITE_BATCH_GROUP_COUNT]);
zfw_bool_t zfw_free_render_layer_sprite_batch_slot(const zfw_sprite_batch_slot_key_t slot_key, zfw_sprite_batch_group_t batch_groups[ZFW_SPRITE_BATCH_GROUP_COUNT]);
zfw_sprite_batch_slot_key_t zfw_create_sprite_batch_slot_key(const int key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_COUNT]);
int zfw_get_sprite_batch_slot_key_elem(const zfw_sprite_batch_slot_key_t slot_key, const zfw_sprite_batch_slot_key_elem_id_t elem_id);
void zfw_init_sprite_batch_slot_key_elems(const zfw_sprite_batch_slot_key_t slot_key, int key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_COUNT]);

zfw_bool_t zfw_init_char_batch_group(zfw_char_batch_group_t *const batch_group, zfw_mem_arena_t *const main_mem_arena);
void zfw_clean_char_batch_group(zfw_char_batch_group_t *const batch_group);
void zfw_set_char_batch_group_defaults(zfw_char_batch_group_t *const batch_group);
zfw_char_batch_key_t zfw_take_render_layer_char_batch(const int layer_index, zfw_char_batch_group_t *const batch_group, zfw_mem_arena_t *const main_mem_arena);
zfw_bool_t zfw_write_to_render_layer_char_batch(const zfw_char_batch_key_t key, const char *const text, const zfw_font_hor_align_t hor_align, const zfw_font_vert_align_t vert_align, zfw_char_batch_group_t *const batch_group, const zfw_user_font_data_t *const user_font_data); // IDEA: Have a faster alternative to this to be used when no alignment is needed.
zfw_bool_t zfw_clear_render_layer_char_batch(const zfw_char_batch_key_t key, zfw_char_batch_group_t *const batch_group);
zfw_bool_t zfw_free_render_layer_char_batch(const zfw_char_batch_key_t key, zfw_char_batch_group_t *const batch_group);
zfw_char_batch_key_t zfw_create_char_batch_key(const int layer_index, const int batch_index);

void zfw_render_sprite_and_character_batches(const zfw_sprite_batch_group_t sprite_batch_groups[ZFW_SPRITE_BATCH_GROUP_COUNT], const zfw_char_batch_group_t *const char_batch_group, const zfw_view_state_t *const view_state, const zfw_vec_2d_i_t window_size, const zfw_user_tex_data_t *const user_tex_data, const zfw_user_font_data_t *const user_font_data, const zfw_builtin_shader_prog_data_t *const builtin_shader_prog_data);

void zfw_set_view_state_defaults(zfw_view_state_t *const view_state);

inline int zfw_get_sprite_batch_group_batch_index(const int layer_index, const int batch_index)
{
    return (layer_index * ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT) + batch_index;
}

inline int zfw_get_sprite_batch_group_slot_index(const int layer_index, const int batch_index, const int slot_index)
{
    return (layer_index * ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_SPRITE_BATCH_SLOT_LIMIT) + (batch_index * ZFW_SPRITE_BATCH_SLOT_LIMIT) + slot_index;
}

inline int zfw_get_sprite_batch_group_tex_unit_index(const int layer_index, const int batch_index, const int tex_unit_index)
{
    return (layer_index * ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT) + (batch_index * ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT) + tex_unit_index;
}

inline zfw_bool_t zfw_is_sprite_batch_slot_key_active(const zfw_sprite_batch_slot_key_t slot_key)
{
    return slot_key & 1;
}

inline int zfw_get_char_batch_group_batch_index(const int layer_index, const int batch_index)
{
    return (layer_index * ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT) + batch_index;
}

inline zfw_bool_t zfw_is_char_batch_slot_key_active(const zfw_char_batch_key_t key)
{
    return key & 1;
}

inline zfw_bool_t zfw_get_char_batch_slot_key_layer_index(const zfw_char_batch_key_t key)
{
    return (key >> 1) & (ZFW_RENDER_LAYER_LIMIT - 1);
}

inline zfw_bool_t zfw_get_char_batch_slot_key_batch_index(const zfw_char_batch_key_t key)
{
    return key >> (1 + (int)log2(ZFW_RENDER_LAYER_LIMIT));
}

inline void zfw_set_render_layer_char_batch_user_font_index(const zfw_char_batch_key_t key, const int user_font_index, zfw_char_batch_group_t *const batch_group)
{
    batch_group->user_font_indexes[zfw_get_char_batch_group_batch_index(zfw_get_char_batch_slot_key_layer_index(key), zfw_get_char_batch_slot_key_batch_index(key))] = user_font_index;
}

inline void zfw_set_render_layer_char_batch_pos(const zfw_char_batch_key_t key, const zfw_vec_2d_t pos, zfw_char_batch_group_t *const batch_group)
{
    batch_group->positions[zfw_get_char_batch_group_batch_index(zfw_get_char_batch_slot_key_layer_index(key), zfw_get_char_batch_slot_key_batch_index(key))] = pos;
}

inline void zfw_set_render_layer_char_batch_scale(const zfw_char_batch_key_t key, const zfw_vec_2d_t scale, zfw_char_batch_group_t *const batch_group)
{
    batch_group->scales[zfw_get_char_batch_group_batch_index(zfw_get_char_batch_slot_key_layer_index(key), zfw_get_char_batch_slot_key_batch_index(key))] = scale;
}

inline void zfw_set_render_layer_char_batch_blend(const zfw_char_batch_key_t key, const zfw_color_t *const blend, zfw_char_batch_group_t *const batch_group)
{
    batch_group->blends[zfw_get_char_batch_group_batch_index(zfw_get_char_batch_slot_key_layer_index(key), zfw_get_char_batch_slot_key_batch_index(key))] = *blend;
}

inline zfw_vec_2d_t zfw_get_view_to_screen_pos(const zfw_vec_2d_t pos, const zfw_view_state_t *const view_state)
{
    return zfw_get_vec_2d_scaled(zfw_create_vec_2d(pos.x - view_state->pos.x, pos.y - view_state->pos.y), view_state->scale);
}

inline zfw_vec_2d_t zfw_get_screen_to_view_pos(const zfw_vec_2d_t pos, const zfw_view_state_t *const view_state)
{
    return zfw_get_vec_2d_sum(view_state->pos, zfw_get_vec_2d_scaled(pos, 1.0f / view_state->scale));
}

#endif
