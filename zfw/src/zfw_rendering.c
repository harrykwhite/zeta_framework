#include <zfw.h>

#include <string.h>
#include <math.h>

static zfw_bool_t init_and_activate_render_layer_sprite_batch(const int layer_index, const int batch_index, zfw_sprite_batch_group_t *const batch_data, zfw_mem_arena_t *const mem_arena)
{
    glBindVertexArray(batch_data->vert_array_gl_ids[layer_index][batch_index]);

    glBindBuffer(GL_ARRAY_BUFFER, batch_data->vert_buf_gl_ids[layer_index][batch_index]);

    {
        float *verts;
        const int verts_size = sizeof(*verts) * ZFW_BUILTIN_TEXTURED_RECT_SHADER_PROG_VERT_COUNT * 4 * ZFW_SPRITE_BATCH_SLOT_LIMIT;
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

    const int verts_stride = sizeof(float) * ZFW_BUILTIN_TEXTURED_RECT_SHADER_PROG_VERT_COUNT;

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

    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, verts_stride, (void *)(sizeof(float) * 10));
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
            return log2(ZFW_SPRITE_BATCH_GROUP_ID_COUNT);

        case ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__LAYER_INDEX:
            return log2(ZFW_RENDER_LAYER_LIMIT);

        case ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_INDEX:
            return log2(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT);

        case ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__SLOT_INDEX:
            return log2(ZFW_SPRITE_BATCH_SLOT_LIMIT);

        case ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__TEX_UNIT_INDEX:
            return log2(ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT);
    }

    return 0;
}

zfw_sprite_batch_slot_key_t zfw_take_slot_from_render_layer_sprite_batch(const zfw_sprite_batch_data_id_t batch_data_id, const int layer_index, const int user_tex_index, zfw_sprite_batch_group_t *const batch_datas, zfw_mem_arena_t *const mem_arena)
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
        int tex_unit_index = -1;

        for (int j = 0; j < tex_unit_limit; j++)
        {
            const zfw_sprite_batch_tex_unit_t tex_unit = batch_datas[batch_data_id].tex_units[layer_index][i][j];

            if (tex_unit.count == 0 || tex_unit.user_tex_index == user_tex_index)
            {
                tex_unit_index = j;
                break;
            }
        }

        if (tex_unit_index == -1)
        {
            continue;
        }

        // Find an available batch slot by searching through the bitset.
        const int slot_activity_bitset_begin_bit_index = (ZFW_SPRITE_BATCH_SLOT_LIMIT * ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * layer_index) + (ZFW_SPRITE_BATCH_SLOT_LIMIT * i);
        const int slot_activity_bitset_first_inactive_bit_index = zfw_get_first_inactive_bitset_bit_index_in_range(&batch_datas[batch_data_id].slot_activity_bitset, slot_activity_bitset_begin_bit_index, slot_activity_bitset_begin_bit_index + ZFW_SPRITE_BATCH_SLOT_LIMIT);

        if (slot_activity_bitset_first_inactive_bit_index != -1)
        {
            // Take the slot and return a key.
            zfw_toggle_bitset_bit(&batch_datas[batch_data_id].slot_activity_bitset, slot_activity_bitset_first_inactive_bit_index, ZFW_TRUE);

            batch_datas[batch_data_id].tex_units[layer_index][i][tex_unit_index].user_tex_index = user_tex_index;
            batch_datas[batch_data_id].tex_units[layer_index][i][tex_unit_index].count++;

            zfw_sprite_batch_slot_key_elems_t slot_key_elems;
            slot_key_elems.active = ZFW_TRUE;
            slot_key_elems.batch_data_index = batch_data_id;
            slot_key_elems.layer_index = layer_index;
            slot_key_elems.batch_index = i;
            slot_key_elems.slot_index = slot_activity_bitset_first_inactive_bit_index - slot_activity_bitset_begin_bit_index;
            slot_key_elems.tex_unit_index = tex_unit_index;

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

void zfw_take_multiple_slots_from_render_layer_sprite_batch(zfw_sprite_batch_slot_key_t *const slot_keys, const int slot_key_count, const zfw_sprite_batch_data_id_t batch_data_id, const int layer_index, const int user_tex_index, zfw_sprite_batch_group_t *const batch_datas, zfw_mem_arena_t *const mem_arena)
{
    if (slot_key_count <= 0)
    {
        zfw_log_warning("Invalid \"slot_key_count\" parameter value (%d) for function \"zfw_take_multiple_slots_from_render_layer_sprite_batch\".", slot_key_count);
        return;
    }

    int tex_unit_limit;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &tex_unit_limit);

    int slots_found_count = 0;
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
        int tex_unit_index = -1;

        for (int j = 0; j < tex_unit_limit; j++)
        {
            const zfw_sprite_batch_tex_unit_t tex_unit = batch_datas[batch_data_id].tex_units[layer_index][i][j];

            if (tex_unit.count == 0 || tex_unit.user_tex_index == user_tex_index)
            {
                tex_unit_index = j;
                break;
            }
        }

        if (tex_unit_index == -1)
        {
            continue;
        }

        // Search for and take inactive batch slots.
        const int slot_activity_bitset_begin_bit_index = (ZFW_SPRITE_BATCH_SLOT_LIMIT * ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * layer_index) + (i * ZFW_SPRITE_BATCH_SLOT_LIMIT);

        for (int j = 0; j < ZFW_SPRITE_BATCH_SLOT_LIMIT; j++)
        {
            if (!zfw_is_bitset_bit_active(&batch_datas[batch_data_id].slot_activity_bitset, slot_activity_bitset_begin_bit_index + j))
            {
                // Take the slot and add a key.
                zfw_toggle_bitset_bit(&batch_datas[batch_data_id].slot_activity_bitset, slot_activity_bitset_begin_bit_index + j, ZFW_TRUE);

                batch_datas[batch_data_id].tex_units[layer_index][i][tex_unit_index].user_tex_index = user_tex_index;
                batch_datas[batch_data_id].tex_units[layer_index][i][tex_unit_index].count++;

                zfw_sprite_batch_slot_key_elems_t slot_key_elems;
                slot_key_elems.active = ZFW_TRUE;
                slot_key_elems.batch_data_index = batch_data_id;
                slot_key_elems.layer_index = layer_index;
                slot_key_elems.batch_index = i;
                slot_key_elems.slot_index = j;
                slot_key_elems.tex_unit_index = tex_unit_index;

                slot_keys[slots_found_count] = zfw_create_sprite_batch_slot_key(&slot_key_elems);

                slots_found_count++;

                if (slots_found_count == slot_key_count)
                {
                    return;
                }
            }
        }
    }

    if (first_inactive_batch_index != -1)
    {
        // Initialize and activate a new sprite batch. If successful, try this all again.
        if (init_and_activate_render_layer_sprite_batch(layer_index, first_inactive_batch_index, &batch_datas[batch_data_id], mem_arena))
        {
            zfw_take_multiple_slots_from_render_layer_sprite_batch(slot_keys + slots_found_count, slot_key_count - slots_found_count, batch_data_id, layer_index, user_tex_index, batch_datas, mem_arena);
        }
    }
}

zfw_bool_t zfw_write_to_render_layer_sprite_batch_slot(const zfw_sprite_batch_slot_key_t slot_key, const zfw_vec_2d_t pos, const float rot, const zfw_vec_2d_t scale, const zfw_vec_2d_t origin, const zfw_rect_t *const src_rect, const zfw_color_t *const blend, const zfw_sprite_batch_group_t *const batch_datas, const zfw_user_tex_data_t *const user_tex_data)
{
    zfw_sprite_batch_slot_key_elems_t slot_key_elems;
    zfw_init_sprite_batch_slot_key_elems(slot_key, &slot_key_elems);

    if (!slot_key_elems.active)
    {
        zfw_log_warning("Attempting to write to a render layer sprite batch slot using an inactive key!");
        return ZFW_FALSE;
    }

    const zfw_sprite_batch_group_t *const batch_data = &batch_datas[slot_key_elems.batch_data_index];

    const int user_tex_index = batch_data->tex_units[slot_key_elems.layer_index][slot_key_elems.batch_index][slot_key_elems.tex_unit_index].user_tex_index;
    const zfw_vec_2d_i_t user_tex_size = user_tex_data->sizes[user_tex_index];

    const float verts[ZFW_BUILTIN_TEXTURED_RECT_SHADER_PROG_VERT_COUNT * 4] = {
        (0.0f - origin.x) * scale.x,
        (0.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        src_rect->width,
        src_rect->height,
        rot,
        slot_key_elems.tex_unit_index,
        (float)src_rect->x / user_tex_size.x,
        (float)src_rect->y / user_tex_size.y,
        blend->r,
        blend->g,
        blend->b,
        blend->a,

        (1.0f - origin.x) * scale.x,
        (0.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        src_rect->width,
        src_rect->height,
        rot,
        slot_key_elems.tex_unit_index,
        (float)(src_rect->x + src_rect->width) / user_tex_size.x,
        (float)src_rect->y / user_tex_size.y,
        blend->r,
        blend->g,
        blend->b,
        blend->a,

        (1.0f - origin.x) * scale.x,
        (1.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        src_rect->width,
        src_rect->height,
        rot,
        slot_key_elems.tex_unit_index,
        (float)(src_rect->x + src_rect->width) / user_tex_size.x,
        (float)(src_rect->y + src_rect->height) / user_tex_size.y,
        blend->r,
        blend->g,
        blend->b,
        blend->a,

        (0.0f - origin.x) * scale.x,
        (1.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        src_rect->width,
        src_rect->height,
        rot,
        slot_key_elems.tex_unit_index,
        (float)src_rect->x / user_tex_size.x,
        (float)(src_rect->y + src_rect->height) / user_tex_size.y,
        blend->r,
        blend->g,
        blend->b,
        blend->a
    };

    glBindVertexArray(batch_data->vert_array_gl_ids[slot_key_elems.layer_index][slot_key_elems.batch_index]);
    glBindBuffer(GL_ARRAY_BUFFER, batch_data->vert_buf_gl_ids[slot_key_elems.layer_index][slot_key_elems.batch_index]);
    glBufferSubData(GL_ARRAY_BUFFER, slot_key_elems.slot_index * sizeof(verts), sizeof(verts), verts);

    return ZFW_TRUE;
}

zfw_bool_t zfw_clear_render_layer_sprite_batch_slot(const zfw_sprite_batch_slot_key_t slot_key, const zfw_sprite_batch_group_t *const batch_datas)
{
    zfw_sprite_batch_slot_key_elems_t slot_key_elems;
    zfw_init_sprite_batch_slot_key_elems(slot_key, &slot_key_elems);

    if (!slot_key_elems.active)
    {
        zfw_log_warning("Attempting to clear a render layer sprite batch slot using an inactive key!");
        return ZFW_FALSE;
    }

    const zfw_sprite_batch_group_t *const batch_data = &batch_datas[slot_key_elems.batch_data_index];

    glBindVertexArray(batch_data->vert_array_gl_ids[slot_key_elems.layer_index][slot_key_elems.batch_index]);
    glBindBuffer(GL_ARRAY_BUFFER, batch_data->vert_buf_gl_ids[slot_key_elems.layer_index][slot_key_elems.batch_index]);

    const float verts[ZFW_BUILTIN_TEXTURED_RECT_SHADER_PROG_VERT_COUNT * 4] = {0};
    glBufferSubData(GL_ARRAY_BUFFER, slot_key_elems.slot_index * sizeof(verts), sizeof(verts), verts);

    return ZFW_TRUE;
}

zfw_bool_t zfw_free_render_layer_sprite_batch_slot(const zfw_sprite_batch_slot_key_t slot_key, zfw_sprite_batch_group_t *const batch_datas)
{
    zfw_sprite_batch_slot_key_elems_t slot_key_elems;
    zfw_init_sprite_batch_slot_key_elems(slot_key, &slot_key_elems);

    if (!slot_key_elems.active)
    {
        zfw_log_warning("Attempting to free a render layer sprite batch slot using an inactive key!");
        return ZFW_FALSE;
    }

    zfw_sprite_batch_group_t *const batch_data = &batch_datas[slot_key_elems.batch_data_index];

    glBindVertexArray(batch_data->vert_array_gl_ids[slot_key_elems.layer_index][slot_key_elems.batch_index]);
    glBindBuffer(GL_ARRAY_BUFFER, batch_data->vert_buf_gl_ids[slot_key_elems.layer_index][slot_key_elems.batch_index]);

    const float verts[ZFW_BUILTIN_TEXTURED_RECT_SHADER_PROG_VERT_COUNT * 4] = {0};
    glBufferSubData(GL_ARRAY_BUFFER, slot_key_elems.slot_index * sizeof(verts), sizeof(verts), verts);

    zfw_toggle_bitset_bit(&batch_data->slot_activity_bitset, (slot_key_elems.layer_index * ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_SPRITE_BATCH_SLOT_LIMIT) + (slot_key_elems.batch_index * ZFW_SPRITE_BATCH_SLOT_LIMIT) + slot_key_elems.slot_index, ZFW_FALSE);

    batch_data->tex_units[slot_key_elems.layer_index][slot_key_elems.batch_index][slot_key_elems.tex_unit_index].count--;

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

void zfw_init_sprite_batch_slot_key_elems(const zfw_sprite_batch_slot_key_t slot_key, zfw_sprite_batch_slot_key_elems_t *const slot_key_elems)
{
    int slot_key_bit_index = 0;

    for (int i = 0; i < ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID_COUNT; i++)
    {
        ((int *)slot_key_elems)[i] = (slot_key >> slot_key_bit_index) & (((zfw_sprite_batch_slot_key_t)1 << get_sprite_batch_slot_key_elem_bit_count(i)) - 1);
        slot_key_bit_index += get_sprite_batch_slot_key_elem_bit_count(i);
    }
}

zfw_char_batch_key_t zfw_take_render_layer_char_batch(const int layer_index, zfw_char_batch_group_t *const batch_data, zfw_mem_arena_t *const mem_arena)
{
    for (int i = 0; i < ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT; i++)
    {
        const int batch_bitmask = (zfw_render_layer_char_batch_bits_t)1 << i;

        if (!(batch_data->batch_activity_bits[layer_index] & batch_bitmask))
        {
            // Initialize the batch if not already done.
            if (!(batch_data->batch_init_bits[layer_index] & batch_bitmask))
            {
                glBindVertexArray(batch_data->vert_array_gl_ids[layer_index][i]);

                glBindBuffer(GL_ARRAY_BUFFER, batch_data->vert_buf_gl_ids[layer_index][i]);

                {
                    float *verts;
                    const int verts_size = sizeof(*verts) * ZFW_BUILTIN_CHAR_RECT_SHADER_PROG_VERT_COUNT * 4 * ZFW_CHAR_BATCH_SLOT_LIMIT;
                    verts = zfw_mem_arena_alloc(mem_arena, verts_size);

                    if (!verts)
                    {
                        zfw_log_error("Failed to allocate %d bytes for render layer character batch vertices!", verts_size);
                        return 0;
                    }

                    memset(verts, 0, verts_size);

                    glBufferData(GL_ARRAY_BUFFER, verts_size, verts, GL_DYNAMIC_DRAW);

                    zfw_rewind_mem_arena(mem_arena);
                }

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch_data->elem_buf_gl_ids[layer_index][i]);

                {
                    unsigned short *indices;
                    const int indices_size = sizeof(*indices) * 6 * ZFW_CHAR_BATCH_SLOT_LIMIT;
                    indices = zfw_mem_arena_alloc(mem_arena, indices_size);

                    if (!indices)
                    {
                        zfw_log_error("Failed to allocate %d bytes for render layer character batch elements!", indices_size);
                        return 0;
                    }

                    for (int i = 0; i < ZFW_CHAR_BATCH_SLOT_LIMIT; i++)
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

                const int verts_stride = sizeof(float) * ZFW_BUILTIN_CHAR_RECT_SHADER_PROG_VERT_COUNT;

                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, verts_stride, (void *)(sizeof(float) * 0));
                glEnableVertexAttribArray(0);

                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, verts_stride, (void *)(sizeof(float) * 2));
                glEnableVertexAttribArray(1);

                glBindVertexArray(0);

                batch_data->batch_init_bits[layer_index] |= batch_bitmask;
            }

            // Take the batch and return a key.
            batch_data->batch_activity_bits[layer_index] |= batch_bitmask;

            zfw_char_batch_key_elems_t key_elems;
            key_elems.active = ZFW_TRUE;
            key_elems.layer_index = layer_index;
            key_elems.batch_index = i;

            return zfw_create_char_batch_key(&key_elems);
        }
    }

    return 0;
}

zfw_bool_t zfw_write_to_render_layer_char_batch(const zfw_char_batch_key_t key, const char *const text, const zfw_font_hor_align_t hor_align, const zfw_font_vert_align_t vert_align, zfw_char_batch_group_t *const batch_group, const zfw_user_font_data_t *const user_font_data)
{
    zfw_char_batch_key_elems_t key_elems;
    zfw_init_char_batch_key_elems(key, &key_elems);

    if (!key_elems.active)
    {
        zfw_log_warning("Attempting to write to a render layer character batch using an inactive key!");
        return ZFW_FALSE;
    }

    const int batch_user_font_index = batch_group->user_font_indexes[key_elems.layer_index][key_elems.batch_index];

    const int text_len = strnlen(text, ZFW_CHAR_BATCH_SLOT_LIMIT);

    if (text_len == 0)
    {
        zfw_log_warning("Attempting to write to a render layer character batch using text with a length of 0!");
        return ZFW_FALSE;
    }

    if (text_len == ZFW_CHAR_BATCH_SLOT_LIMIT && text[text_len])
    {
        zfw_log_warning("Attempting to write to a render layer character batch using text of a length exceeding the limit of %d!", ZFW_CHAR_BATCH_SLOT_LIMIT);
        return ZFW_FALSE;
    }

    // Determine the positions of text characters based on font information, alongside the overall dimensions of the text to be used when applying alignment.
    zfw_vec_2d_t char_draw_positions[ZFW_CHAR_BATCH_SLOT_LIMIT];
    zfw_vec_2d_t char_draw_pos_pen = {0};

    int text_line_widths[ZFW_CHAR_BATCH_SLOT_LIMIT] = {0};
    int text_first_line_min_offs;
    int text_first_line_min_offs_updated = ZFW_FALSE;
    int text_last_line_max_height;
    int text_last_line_max_height_updated = ZFW_FALSE;
    int text_line_counter = 0;

    for (int i = 0; i < text_len; i++)
    {
        if (text[i] == '\n')
        {
            text_line_widths[text_line_counter] = char_draw_pos_pen.x;

            if (!text_first_line_min_offs_updated)
            {
                // Set the first line minimum offset to the vertical offset of the space character.
                // IDEA: Translate the above comment into code.
                text_first_line_min_offs = user_font_data->chars_vert_offsets[batch_user_font_index * ZFW_FONT_CHAR_RANGE_SIZE];
                text_first_line_min_offs_updated = ZFW_TRUE;
            }

            // Set the last line maximum height to the height of a space.
            text_last_line_max_height = user_font_data->chars_vert_offsets[batch_user_font_index * ZFW_FONT_CHAR_RANGE_SIZE] + user_font_data->chars_src_rects[batch_user_font_index * ZFW_FONT_CHAR_RANGE_SIZE].height;
            //

            text_last_line_max_height_updated = ZFW_FALSE;

            text_line_counter++;

            // Move the pen to a new line.
            char_draw_pos_pen.x = 0.0f;
            char_draw_pos_pen.y += user_font_data->line_heights[batch_user_font_index];
            //

            continue;
        }

        const int text_char_index = text[i] - ZFW_FONT_CHAR_RANGE_BEGIN;
        const int user_font_chars_index = (batch_user_font_index * ZFW_FONT_CHAR_RANGE_SIZE) + text_char_index;

        // If we are on the first line, update the first line minimum offset.
        if (text_line_counter == 0)
        {
            if (!text_first_line_min_offs_updated)
            {
                text_first_line_min_offs = user_font_data->chars_vert_offsets[user_font_chars_index];
                text_first_line_min_offs_updated = ZFW_TRUE;
            }
            else
            {
                text_first_line_min_offs = ZFW_MIN(user_font_data->chars_vert_offsets[user_font_chars_index], text_first_line_min_offs);
            }
        }

        if (!text_last_line_max_height_updated)
        {
            text_last_line_max_height = user_font_data->chars_vert_offsets[user_font_chars_index] + user_font_data->chars_src_rects[user_font_chars_index].height;
            text_last_line_max_height_updated = ZFW_TRUE;
        }
        else
        {
            text_last_line_max_height = ZFW_MAX(user_font_data->chars_vert_offsets[user_font_chars_index] + user_font_data->chars_src_rects[user_font_chars_index].height, text_last_line_max_height);
        }

        if (i > 0)
        {
            // Apply kerning based on the previous character.
            const int text_char_index_last = text[i - 1] - ZFW_FONT_CHAR_RANGE_BEGIN;
            char_draw_pos_pen.x += user_font_data->chars_kernings[(batch_user_font_index * ZFW_FONT_CHAR_RANGE_SIZE * ZFW_FONT_CHAR_RANGE_SIZE) + (text_char_index * ZFW_FONT_CHAR_RANGE_SIZE) + text_char_index_last];
        }

        char_draw_positions[i].x = char_draw_pos_pen.x + user_font_data->chars_hor_offsets[user_font_chars_index];
        char_draw_positions[i].y = char_draw_pos_pen.y + user_font_data->chars_vert_offsets[user_font_chars_index];

        char_draw_pos_pen.x += user_font_data->chars_hor_advances[user_font_chars_index];
    }

    text_line_widths[text_line_counter] = char_draw_pos_pen.x;
    text_line_counter = 0;

    const int text_height = text_first_line_min_offs + char_draw_pos_pen.y + text_last_line_max_height;

    // Clear the batch then write the character render data.
    glBindVertexArray(batch_group->vert_array_gl_ids[key_elems.layer_index][key_elems.batch_index]);
    glBindBuffer(GL_ARRAY_BUFFER, batch_group->vert_buf_gl_ids[key_elems.layer_index][key_elems.batch_index]);

    {
        const float batch_clear_verts[(ZFW_BUILTIN_CHAR_RECT_SHADER_PROG_VERT_COUNT * 4) * ZFW_CHAR_BATCH_SLOT_LIMIT] = {0};
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(batch_clear_verts), batch_clear_verts);
    }

    for (int i = 0; i < text_len; i++)
    {
        if (text[i] == '\n')
        {
            text_line_counter++;
            continue;
        }

        if (text[i] != ' ')
        {
            const int font_chars_index = (batch_user_font_index * ZFW_FONT_CHAR_RANGE_SIZE) + (text[i] - ZFW_FONT_CHAR_RANGE_BEGIN);

            const zfw_vec_2d_t char_draw_pos = zfw_create_vec_2d(char_draw_positions[i].x - (text_line_widths[text_line_counter] * (hor_align / 2.0f)), char_draw_positions[i].y - (text_height * (vert_align / 2.0f)));

            zfw_rect_f_t char_tex_coords_rect;
            char_tex_coords_rect.x = (float)user_font_data->chars_src_rects[font_chars_index].x / user_font_data->tex_sizes[batch_user_font_index].x;
            char_tex_coords_rect.y = (float)user_font_data->chars_src_rects[font_chars_index].y / user_font_data->tex_sizes[batch_user_font_index].y;
            char_tex_coords_rect.width = (float)user_font_data->chars_src_rects[font_chars_index].width / user_font_data->tex_sizes[batch_user_font_index].x;
            char_tex_coords_rect.height = (float)user_font_data->chars_src_rects[font_chars_index].height / user_font_data->tex_sizes[batch_user_font_index].y;

            const float verts[ZFW_BUILTIN_CHAR_RECT_SHADER_PROG_VERT_COUNT * 4] = {
                char_draw_pos.x,
                char_draw_pos.y,
                char_tex_coords_rect.x,
                char_tex_coords_rect.y,

                char_draw_pos.x + user_font_data->chars_src_rects[font_chars_index].width,
                char_draw_pos.y,
                char_tex_coords_rect.x + char_tex_coords_rect.width,
                char_tex_coords_rect.y,

                char_draw_pos.x + user_font_data->chars_src_rects[font_chars_index].width,
                char_draw_pos.y + user_font_data->chars_src_rects[font_chars_index].height,
                char_tex_coords_rect.x + char_tex_coords_rect.width,
                char_tex_coords_rect.y + char_tex_coords_rect.height,

                char_draw_pos.x,
                char_draw_pos.y + user_font_data->chars_src_rects[font_chars_index].height,
                char_tex_coords_rect.x,
                char_tex_coords_rect.y + char_tex_coords_rect.height
            };

            glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(verts), sizeof(verts), verts);
        }
    }

    return ZFW_TRUE;
}

zfw_bool_t zfw_set_render_layer_char_batch_user_font_index(const zfw_char_batch_key_t key, const int user_font_index, zfw_char_batch_group_t *const batch_data)
{
    zfw_char_batch_key_elems_t key_elems;
    zfw_init_char_batch_key_elems(key, &key_elems);

    if (!key_elems.active)
    {
        zfw_log_warning("Attempting to set the user font index of a render layer character batch using an inactive key!");
        return ZFW_FALSE;
    }

    batch_data->user_font_indexes[key_elems.layer_index][key_elems.batch_index] = user_font_index;

    return ZFW_TRUE;
}

zfw_bool_t zfw_set_render_layer_char_batch_pos(const zfw_char_batch_key_t key, const zfw_vec_2d_t pos, zfw_char_batch_group_t *const batch_data)
{
    zfw_char_batch_key_elems_t key_elems;
    zfw_init_char_batch_key_elems(key, &key_elems);

    if (!key_elems.active)
    {
        zfw_log_warning("Attempting to set the position of a render layer character batch using an inactive key!");
        return ZFW_FALSE;
    }

    batch_data->positions[key_elems.layer_index][key_elems.batch_index] = pos;

    return ZFW_TRUE;
}

zfw_bool_t zfw_set_render_layer_char_batch_scale(const zfw_char_batch_key_t key, const zfw_vec_2d_t scale, zfw_char_batch_group_t *const batch_data)
{
    zfw_char_batch_key_elems_t key_elems;
    zfw_init_char_batch_key_elems(key, &key_elems);

    if (!key_elems.active)
    {
        zfw_log_warning("Attempting to set the scale of a render layer character batch using an inactive key!");
        return ZFW_FALSE;
    }

    batch_data->scales[key_elems.layer_index][key_elems.batch_index] = scale;

    return ZFW_TRUE;
}

zfw_bool_t zfw_set_render_layer_char_batch_blend(const zfw_char_batch_key_t key, const zfw_color_t *const blend, zfw_char_batch_group_t *const batch_data)
{
    zfw_char_batch_key_elems_t key_elems;
    zfw_init_char_batch_key_elems(key, &key_elems);

    if (!key_elems.active)
    {
        zfw_log_warning("Attempting to set the blend of a render layer character batch using an inactive key!");
        return ZFW_FALSE;
    }

    batch_data->blends[key_elems.layer_index][key_elems.batch_index] = *blend;

    return ZFW_TRUE;
}

zfw_bool_t zfw_clear_render_layer_char_batch(const zfw_char_batch_key_t key, zfw_char_batch_group_t *const batch_data)
{
    zfw_char_batch_key_elems_t key_elems;
    zfw_init_char_batch_key_elems(key, &key_elems);

    if (!key_elems.active)
    {
        zfw_log_warning("Attempting to clear a render layer character batch using an inactive key!");
        return ZFW_FALSE;
    }

    glBindVertexArray(batch_data->vert_array_gl_ids[key_elems.layer_index][key_elems.batch_index]);
    glBindBuffer(GL_ARRAY_BUFFER, batch_data->vert_buf_gl_ids[key_elems.layer_index][key_elems.batch_index]);

    const float verts[(ZFW_BUILTIN_CHAR_RECT_SHADER_PROG_VERT_COUNT * 4) * ZFW_CHAR_BATCH_SLOT_LIMIT] = {0};
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    return ZFW_TRUE;
}

zfw_bool_t zfw_free_render_layer_char_batch(const zfw_char_batch_key_t key, zfw_char_batch_group_t *const batch_data)
{
    zfw_char_batch_key_elems_t key_elems;
    zfw_init_char_batch_key_elems(key, &key_elems);

    if (!key_elems.active)
    {
        zfw_log_warning("Attempting to free a render layer character batch using an inactive key!");
        return ZFW_FALSE;
    }

    glBindVertexArray(batch_data->vert_array_gl_ids[key_elems.layer_index][key_elems.batch_index]);
    glBindBuffer(GL_ARRAY_BUFFER, batch_data->vert_buf_gl_ids[key_elems.layer_index][key_elems.batch_index]);

    const float verts[(ZFW_BUILTIN_CHAR_RECT_SHADER_PROG_VERT_COUNT * 4) * ZFW_CHAR_BATCH_SLOT_LIMIT] = {0};
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    batch_data->batch_activity_bits[key_elems.layer_index] &= ~((zfw_render_layer_char_batch_bits_t)1 << key_elems.batch_index);

    return ZFW_TRUE;
}

zfw_char_batch_key_t zfw_create_char_batch_key(const zfw_char_batch_key_elems_t *const key_elems)
{
    zfw_char_batch_key_t key = key_elems->active;
    key |= key_elems->layer_index << 1;
    key |= key_elems->batch_index << (1 + (int)log2(ZFW_RENDER_LAYER_LIMIT));

    return key;
}

void zfw_init_char_batch_key_elems(const zfw_sprite_batch_slot_key_t key, zfw_char_batch_key_elems_t *const key_elems)
{
    key_elems->active = key & 1;
    key_elems->layer_index = (key >> 1) & (ZFW_RENDER_LAYER_LIMIT - 1);
    key_elems->batch_index = key >> (1 + (int)log2(ZFW_RENDER_LAYER_LIMIT));
}

void zfw_reset_view_state(zfw_view_state_t *const view_state)
{
    view_state->pos = zfw_create_vec_2d(0.0f, 0.0f);
    view_state->scale = 1.0f;
}

zfw_vec_2d_t zfw_get_view_to_screen_pos(const zfw_vec_2d_t pos, const zfw_view_state_t *const view_state)
{
    return zfw_get_vec_2d_scaled(zfw_create_vec_2d(pos.x - view_state->pos.x, pos.y - view_state->pos.y), view_state->scale);
}

zfw_vec_2d_t zfw_get_screen_to_view_pos(const zfw_vec_2d_t pos, const zfw_view_state_t *const view_state)
{
    return zfw_get_vec_2d_sum(view_state->pos, zfw_get_vec_2d_scaled(pos, 1.0f / view_state->scale));
}
