#include <string.h>
#include <math.h>
#include "zfw.h"

static zfw_bool_t init_and_activate_render_layer_sprite_batch(const int layer_index, const int batch_index, zfw_sprite_batch_data_t *const batch_data, zfw_mem_arena_t *const mem_arena)
{
    glBindVertexArray(batch_data->vert_array_gl_ids[layer_index][batch_index]);

    glBindBuffer(GL_ARRAY_BUFFER, batch_data->vert_buf_gl_ids[layer_index][batch_index]);

    {
        float *verts;
        const int verts_size = sizeof(*verts) * 44 * ZFW_SPRITE_BATCH_SLOT_LIMIT;
        verts = zfw_mem_arena_alloc(mem_arena, verts_size);

        if (!verts)
        {
            zfw_log_error("Failed to allocate %d bytes for render layer sprite batch vertices!", verts_size);
            return ZFW_FALSE;
        }

        memset(verts, 0, verts_size);

        glBufferData(GL_ARRAY_BUFFER, verts_size, verts, GL_DYNAMIC_DRAW);

        zfw_rewind_mem_arena(mem_arena);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch_data->elem_buf_gl_ids[layer_index][batch_index]);

    {
        unsigned short *indices;
        const int indices_size = sizeof(*indices) * 6 * ZFW_SPRITE_BATCH_SLOT_LIMIT;
        indices = zfw_mem_arena_alloc(mem_arena, indices_size);

        if (!indices)
        {
            zfw_log_error("Failed to allocate %d bytes for render layer sprite batch elements!", indices_size);
            return ZFW_FALSE;
        }

        for (int i = 0; i < ZFW_SPRITE_BATCH_SLOT_LIMIT; i++)
        {
            indices[(i * 6) + 0] = (i * 4) + 0;
            indices[(i * 6) + 1] = (i * 4) + 1;
            indices[(i * 6) + 2] = (i * 4) + 2;
            indices[(i * 6) + 3] = (i * 4) + 2;
            indices[(i * 6) + 4] = (i * 4) + 3;
            indices[(i * 6) + 5] = (i * 4) + 0;
        }

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, indices, GL_STATIC_DRAW);

        zfw_rewind_mem_arena(mem_arena);
    }

    const int verts_stride = sizeof(float) * 11;

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, verts_stride, (void *)(sizeof(float) * 0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, verts_stride, (void *)(sizeof(float) * 2));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, verts_stride, (void *)(sizeof(float) * 4));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, verts_stride, (void *)(sizeof(float) * 6));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, verts_stride, (void *)(sizeof(float) * 7));
    glEnableVertexAttribArray(4);

    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, verts_stride, (void *)(sizeof(float) * 8));
    glEnableVertexAttribArray(5);

    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, verts_stride, (void *)(sizeof(float) * 10));
    glEnableVertexAttribArray(6);

    glBindVertexArray(0);

    batch_data->batch_activity_bits[layer_index] |= (zfw_render_layer_sprite_batch_activity_bits_t)1 << batch_index;

    return ZFW_TRUE;
}

static int get_sprite_batch_slot_key_elem_bit_count(const zfw_sprite_batch_slot_key_elem_id_t slot_key_elem_id)
{
    switch (slot_key_elem_id)
    {
        case ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__ACTIVE:
            return 1;

        case ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_DATA_INDEX:
            return log2(ZFW_SPRITE_BATCH_DATA_ID_COUNT);

        case ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__LAYER_INDEX:
            return log2(ZFW_RENDER_LAYER_LIMIT);

        case ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_INDEX:
            return log2(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT);

        case ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__SLOT_INDEX:
            return log2(ZFW_SPRITE_BATCH_SLOT_LIMIT);

        case ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__TEX_UNIT:
            return log2(ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT);
    }

    return 0;
}

zfw_sprite_batch_slot_key_t zfw_take_slot_from_render_layer_sprite_batch(const zfw_sprite_batch_data_id_t batch_data_id, const int layer_index, const int user_tex_index, zfw_sprite_batch_data_t *const batch_datas, zfw_mem_arena_t *const mem_arena)
{
    int tex_unit_limit;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &tex_unit_limit);

    int first_inactive_batch_index = -1;

    for (int i = 0; i < ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT; i++)
    {
        // Check if the batch is active.
        if (!(batch_datas[batch_data_id].batch_activity_bits[layer_index] & ((zfw_render_layer_sprite_batch_activity_bits_t)1 << i)))
        {
            if (first_inactive_batch_index == -1)
            {
                first_inactive_batch_index = i;
            }

            continue;
        }

        // Find a texture unit to use.
        int tex_unit = -1;

        for (int j = 0; j < tex_unit_limit; j++)
        {
            if (batch_datas[batch_data_id].user_tex_indexes[layer_index][i][j] == -1 || batch_datas[batch_data_id].user_tex_indexes[layer_index][i][j] == user_tex_index)
            {
                tex_unit = j;
                break;
            }
        }

        if (tex_unit == -1)
        {
            continue;
        }

        // Find an available batch slot.
        const int slot_activity_bitset_begin_index = (ZFW_SPRITE_BATCH_SLOT_LIMIT * ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * layer_index) + (ZFW_SPRITE_BATCH_SLOT_LIMIT * i);
        const int slot_index = zfw_get_first_inactive_bitset_bit_index_in_range(&batch_datas[batch_data_id].slot_activity_bitset, slot_activity_bitset_begin_index, slot_activity_bitset_begin_index + ZFW_SPRITE_BATCH_SLOT_LIMIT);

        if (slot_index != -1)
        {
            // Take the slot and return a key.
            zfw_toggle_bitset_bit(&batch_datas[batch_data_id].slot_activity_bitset, slot_activity_bitset_begin_index + slot_index, ZFW_TRUE);
            batch_datas[batch_data_id].user_tex_indexes[layer_index][i][tex_unit] = user_tex_index;

            zfw_sprite_batch_slot_key_elems_t slot_key_elems;
            slot_key_elems.active = ZFW_TRUE;
            slot_key_elems.batch_data_index = batch_data_id;
            slot_key_elems.layer_index = layer_index;
            slot_key_elems.batch_index = i;
            slot_key_elems.slot_index = slot_index;
            slot_key_elems.tex_unit = tex_unit;

            return zfw_create_sprite_batch_slot_key(&slot_key_elems);
        }
    }

    if (first_inactive_batch_index != -1)
    {
        // Initialize and activate a new sprite batch. If successful, try this all again.
        if (init_and_activate_render_layer_sprite_batch(layer_index, first_inactive_batch_index, &batch_datas[batch_data_id], mem_arena))
        {
            return zfw_take_slot_from_render_layer_sprite_batch(batch_data_id, layer_index, user_tex_index, batch_datas, mem_arena);
        }
    }

    // Return an inactive slot key.
    return 0;
}

zfw_bool_t zfw_write_to_render_layer_sprite_batch_slot(const zfw_sprite_batch_slot_key_t slot_key, const zfw_vec_2d_t pos, const float rot, const zfw_vec_2d_t scale, const zfw_vec_2d_t origin, const zfw_rect_t *const src_rect, const float opacity, const zfw_sprite_batch_data_t *const batch_datas, const zfw_user_tex_data_t *const user_tex_data)
{
    if (!(slot_key & 1))
    {
        zfw_log_error("Attempting to write to a render layer sprite batch slot using an inactive key!");
        return ZFW_FALSE;
    }

    zfw_sprite_batch_slot_key_elems_t slot_key_elems;
    zfw_get_sprite_batch_slot_key_elems(slot_key, &slot_key_elems);

    const zfw_sprite_batch_data_t *const batch_data = &batch_datas[slot_key_elems.batch_data_index];

    const int user_tex_index = batch_data->user_tex_indexes[slot_key_elems.layer_index][slot_key_elems.batch_index][slot_key_elems.tex_unit];
    const zfw_vec_2d_i_t user_tex_size = user_tex_data->sizes[user_tex_index];

    const float verts[] = {
        (0.0f - origin.x) * scale.x,
        (0.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        src_rect->width,
        src_rect->height,
        rot,
        slot_key_elems.tex_unit,
        (float)src_rect->x / user_tex_size.x,
        (float)src_rect->y / user_tex_size.y,
        opacity,

        (1.0f - origin.x) * scale.x,
        (0.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        src_rect->width,
        src_rect->height,
        rot,
        slot_key_elems.tex_unit,
        (float)(src_rect->x + src_rect->width) / user_tex_size.x,
        (float)src_rect->y / user_tex_size.y,
        opacity,

        (1.0f - origin.x) * scale.x,
        (1.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        src_rect->width,
        src_rect->height,
        rot,
        slot_key_elems.tex_unit,
        (float)(src_rect->x + src_rect->width) / user_tex_size.x,
        (float)(src_rect->y + src_rect->height) / user_tex_size.y,
        opacity,

        (0.0f - origin.x) * scale.x,
        (1.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        src_rect->width,
        src_rect->height,
        rot,
        slot_key_elems.tex_unit,
        (float)src_rect->x / user_tex_size.x,
        (float)(src_rect->y + src_rect->height) / user_tex_size.y,
        opacity
    };

    glBindVertexArray(batch_data->vert_array_gl_ids[slot_key_elems.layer_index][slot_key_elems.batch_index]);
    glBindBuffer(GL_ARRAY_BUFFER, batch_data->vert_buf_gl_ids[slot_key_elems.layer_index][slot_key_elems.batch_index]);
    glBufferSubData(GL_ARRAY_BUFFER, slot_key_elems.slot_index * sizeof(verts), sizeof(verts), verts);

    return ZFW_TRUE;
}

zfw_sprite_batch_slot_key_t zfw_create_sprite_batch_slot_key(const zfw_sprite_batch_slot_key_elems_t *const slot_key_elems)
{
    zfw_sprite_batch_slot_key_t slot_key = 0;
    int slot_key_bit_index = 0;

    for (int i = 0; i < ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID_COUNT; i++)
    {
        slot_key |= ((int *)slot_key_elems)[i] << slot_key_bit_index;
        slot_key_bit_index += get_sprite_batch_slot_key_elem_bit_count(i);
    }

    return slot_key;
}

void zfw_get_sprite_batch_slot_key_elems(const zfw_sprite_batch_slot_key_t slot_key, zfw_sprite_batch_slot_key_elems_t *const slot_key_elems)
{
    int slot_key_bit_index = 0;

    for (int i = 0; i < ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID_COUNT; i++)
    {
        ((int *)slot_key_elems)[i] = (slot_key >> slot_key_bit_index) & (((zfw_sprite_batch_slot_key_t)1 << get_sprite_batch_slot_key_elem_bit_count(i)) - 1);
        slot_key_bit_index += get_sprite_batch_slot_key_elem_bit_count(i);
    }
}

zfw_vec_2d_t zfw_get_view_to_screen_pos(const zfw_vec_2d_t pos, const zfw_view_state_t *const view_state)
{
    return zfw_get_vec_2d_scaled(zfw_create_vec_2d(pos.x - view_state->pos.x, pos.y - view_state->pos.y), view_state->scale);
}

zfw_vec_2d_t zfw_get_screen_to_view_pos(const zfw_vec_2d_t pos, const zfw_view_state_t *const view_state)
{
    return zfw_get_vec_2d_sum(view_state->pos, zfw_get_vec_2d_scaled(pos, 1.0f / view_state->scale));
}
