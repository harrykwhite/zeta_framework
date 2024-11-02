#include <zfw_rendering.h>

#include <string.h>
#include <zfw_common_debug.h>

const zfw_color_t zfw_k_color_white = {1.0f, 1.0f, 1.0f, 1.0f};
const zfw_color_t zfw_k_color_black = {0.0f, 0.0f, 0.0f, 1.0f};
const zfw_color_t zfw_k_color_red = {1.0f, 0.0f, 0.0f, 1.0f};
const zfw_color_t zfw_k_color_green = {0.0f, 1.0f, 0.0f, 1.0f};
const zfw_color_t zfw_k_color_blue = {0.0f, 0.0f, 1.0f, 1.0f};

static int get_gl_tex_unit_limit()
{
    int limit;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &limit);
    return limit;
}

static zfw_bool_t init_and_activate_render_layer_sprite_batch(const int layer_index, const int batch_index, zfw_sprite_batch_group_t *const batch_group, zfw_mem_arena_t *const main_mem_arena)
{
    const int batch_group_batch_index = zfw_get_sprite_batch_group_batch_index(layer_index, batch_index);

    glBindVertexArray(batch_group->vert_array_gl_ids[batch_group_batch_index]);

    glBindBuffer(GL_ARRAY_BUFFER, batch_group->vert_buf_gl_ids[batch_group_batch_index]);

    {
        float *verts;
        const int verts_size = sizeof(*verts) * ZFW_BUILTIN_SPRITE_QUAD_SHADER_PROG_VERT_COUNT * 4 * ZFW_SPRITE_BATCH_SLOT_LIMIT;
        verts = zfw_mem_arena_alloc(main_mem_arena, verts_size);

        if (!verts)
        {
            zfw_log_error("Failed to allocate %d bytes for render layer sprite batch vertices!", verts_size);
            return ZFW_FALSE;
        }

        memset(verts, 0, verts_size);

        glBufferData(GL_ARRAY_BUFFER, verts_size, verts, GL_DYNAMIC_DRAW);

        zfw_rewind_mem_arena(main_mem_arena);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch_group->elem_buf_gl_ids[batch_group_batch_index]);

    {
        unsigned short *indices;
        const int indices_size = sizeof(*indices) * 6 * ZFW_SPRITE_BATCH_SLOT_LIMIT;
        indices = zfw_mem_arena_alloc(main_mem_arena, indices_size);

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

        zfw_rewind_mem_arena(main_mem_arena);
    }

    const int verts_stride = sizeof(float) * ZFW_BUILTIN_SPRITE_QUAD_SHADER_PROG_VERT_COUNT;

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

    batch_group->batch_activity_bits[layer_index] |= (zfw_render_layer_sprite_batch_activity_bits_t)1 << batch_index;

    return ZFW_TRUE;
}

static int get_sprite_batch_slot_key_elem_bit_count(const zfw_sprite_batch_slot_key_elem_id_t key_elem_id)
{
    switch (key_elem_id)
    {
        case ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_GROUP_INDEX: return log2(ZFW_SPRITE_BATCH_GROUP_COUNT);
        case ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__LAYER_INDEX: return log2(ZFW_RENDER_LAYER_LIMIT);
        case ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_INDEX: return log2(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT);
        case ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__SLOT_INDEX: return log2(ZFW_SPRITE_BATCH_SLOT_LIMIT);
        case ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__TEX_UNIT_INDEX: return log2(ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT);
    }

    return -1;
}

static void draw_sprite_batches_of_layer(const zfw_sprite_batch_group_t *const batch_group, const int layer_index, const zfw_user_tex_data_t *const user_tex_data, const zfw_builtin_shader_prog_data_t *const builtin_shader_prog_data)
{
    if (!batch_group->batch_activity_bits[layer_index])
    {
        // All the batches of this layer are inactive, so don't bother proceeding.
        return;
    }

    for (int i = 0; i < ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT; i++)
    {
        if (!(batch_group->batch_activity_bits[layer_index] & ((zfw_render_layer_sprite_batch_activity_bits_t)1 << i)))
        {
            continue;
        }

        int tex_units[ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT] = {0};

        for (int j = 0; j < ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT; j++)
        {
            const int batch_group_tex_unit_index = zfw_get_sprite_batch_group_tex_unit_index(layer_index, i, j);

            if (batch_group->tex_units[batch_group_tex_unit_index].count)
            {
                tex_units[j] = j;

                glActiveTexture(GL_TEXTURE0 + j);
                glBindTexture(GL_TEXTURE_2D, user_tex_data->gl_ids[batch_group->tex_units[batch_group_tex_unit_index].user_tex_index]);
            }
        }

        glUniform1iv(glGetUniformLocation(builtin_shader_prog_data->sprite_quad_prog_gl_id, "u_textures"), ZFW_STATIC_ARRAY_LEN(tex_units), tex_units);

        glBindVertexArray(batch_group->vert_array_gl_ids[zfw_get_sprite_batch_group_batch_index(layer_index, i)]);
        glDrawElements(GL_TRIANGLES, 6 * ZFW_SPRITE_BATCH_SLOT_LIMIT, GL_UNSIGNED_SHORT, 0);
    }
}

static void draw_char_batches_of_layer(const zfw_char_batch_group_t *const batch_group, const int layer_index, const zfw_user_font_data_t *const user_font_data, const zfw_builtin_shader_prog_data_t *const builtin_shader_prog_data)
{
    if (!batch_group->batch_activity_bits[layer_index])
    {
        // All the batches of this layer are inactive, so don't bother proceeding.
        return;
    }

    for (int i = 0; i < ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT; i++)
    {
        if (!(batch_group->batch_activity_bits[layer_index] & ((zfw_render_layer_char_batch_bits_t)1 << i)))
        {
            continue;
        }

        const int batch_group_batch_index = zfw_get_char_batch_group_batch_index(layer_index, i);

        glUniform2fv(glGetUniformLocation(builtin_shader_prog_data->char_quad_prog_gl_id, "u_pos"), 1, (float *)&batch_group->positions[batch_group_batch_index]);
        glUniform2fv(glGetUniformLocation(builtin_shader_prog_data->char_quad_prog_gl_id, "u_scale"), 1, (float *)&batch_group->scales[batch_group_batch_index]);
        glUniform4fv(glGetUniformLocation(builtin_shader_prog_data->char_quad_prog_gl_id, "u_blend"), 1, (float *)&batch_group->blends[batch_group_batch_index]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, user_font_data->tex_gl_ids[batch_group->user_font_indexes[batch_group_batch_index]]);

        glBindVertexArray(batch_group->vert_array_gl_ids[batch_group_batch_index]);
        glDrawElements(GL_TRIANGLES, 6 * ZFW_CHAR_BATCH_SLOT_LIMIT, GL_UNSIGNED_SHORT, 0);
    }
}

zfw_bool_t zfw_init_sprite_batch_group(zfw_sprite_batch_group_t *const batch_group, zfw_mem_arena_t *const main_mem_arena)
{
    memset(batch_group, 0, sizeof(*batch_group));

    const int batch_group_batch_count = ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT;

    // Generate vertex arrays after allocating memory for OpenGL IDs.
    batch_group->vert_array_gl_ids = zfw_mem_arena_alloc(main_mem_arena, sizeof(*batch_group->vert_array_gl_ids) * batch_group_batch_count);

    if (!batch_group->vert_array_gl_ids)
    {
        return ZFW_FALSE;
    }

    glGenVertexArrays(batch_group_batch_count, batch_group->vert_array_gl_ids);

    // Generate vertex buffers after allocating memory for OpenGL IDs.
    batch_group->vert_buf_gl_ids = zfw_mem_arena_alloc(main_mem_arena, sizeof(*batch_group->vert_buf_gl_ids) * batch_group_batch_count);

    if (!batch_group->vert_buf_gl_ids)
    {
        return ZFW_FALSE;
    }

    glGenBuffers(batch_group_batch_count, batch_group->vert_buf_gl_ids);

    // Generate element buffers after allocating memory for OpenGL IDs.
    batch_group->elem_buf_gl_ids = zfw_mem_arena_alloc(main_mem_arena, sizeof(*batch_group->elem_buf_gl_ids) * batch_group_batch_count);

    if (!batch_group->elem_buf_gl_ids)
    {
        return ZFW_FALSE;
    }

    glGenBuffers(batch_group_batch_count, batch_group->elem_buf_gl_ids);

    // Allocate memory for texture units.
    batch_group->tex_units = zfw_mem_arena_alloc(main_mem_arena, sizeof(*batch_group->tex_units) * ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT * batch_group_batch_count);

    if (!batch_group->tex_units)
    {
        return ZFW_FALSE;
    }

    // Initialise the slot activity bitset.
    zfw_init_bitset_in_mem_arena(&batch_group->slot_activity, ZFW_SPRITE_BATCH_SLOT_LIMIT * ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, main_mem_arena);

    return ZFW_TRUE;
}

void zfw_clean_sprite_batch_group(zfw_sprite_batch_group_t *const batch_group)
{
    if (batch_group->elem_buf_gl_ids)
    {
        glDeleteBuffers(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, batch_group->elem_buf_gl_ids);
    }

    if (batch_group->vert_buf_gl_ids)
    {
        glDeleteBuffers(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, batch_group->vert_buf_gl_ids);
    }

    if (batch_group->vert_array_gl_ids)
    {
        glDeleteVertexArrays(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, batch_group->vert_array_gl_ids);
    }
}

void zfw_set_sprite_batch_group_defaults(zfw_sprite_batch_group_t *const batch_group)
{
    memset(batch_group->batch_activity_bits, 0, sizeof(batch_group->batch_activity_bits));
    memset(batch_group->tex_units, 0, sizeof(*batch_group->tex_units) * ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT * ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT);
    zfw_clear_bitset(&batch_group->slot_activity);
}

zfw_sprite_batch_slot_key_t zfw_take_render_layer_sprite_batch_slot(const zfw_sprite_batch_group_id_t batch_group_id, const int layer_index, const int user_tex_index, zfw_sprite_batch_group_t batch_groups[ZFW_SPRITE_BATCH_GROUP_COUNT], zfw_mem_arena_t *const main_mem_arena)
{
    int first_inactive_batch_index = -1;

    for (int i = 0; i < ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT; i++)
    {
        // Check if the batch is active.
        if (!(batch_groups[batch_group_id].batch_activity_bits[layer_index] & ((zfw_render_layer_sprite_batch_activity_bits_t)1 << i)))
        {
            if (first_inactive_batch_index == -1)
            {
                first_inactive_batch_index = i;
            }

            continue;
        }

        // Find a texture unit to use.
        int tex_unit_index = -1;

        for (int j = 0; j < get_gl_tex_unit_limit(); j++)
        {
            const zfw_sprite_batch_tex_unit_t tex_unit = batch_groups[batch_group_id].tex_units[zfw_get_sprite_batch_group_tex_unit_index(layer_index, i, j)];

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
        const int slot_activity_bitset_begin_bit_index = (layer_index * ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_SPRITE_BATCH_SLOT_LIMIT) + (i * ZFW_SPRITE_BATCH_SLOT_LIMIT);
        const int slot_activity_bitset_first_inactive_bit_index = zfw_get_first_inactive_bitset_bit_index_in_range(&batch_groups[batch_group_id].slot_activity, slot_activity_bitset_begin_bit_index, slot_activity_bitset_begin_bit_index + ZFW_SPRITE_BATCH_SLOT_LIMIT);

        if (slot_activity_bitset_first_inactive_bit_index != -1)
        {
            // Take the slot and return a key.
            zfw_activate_bitset_bit(&batch_groups[batch_group_id].slot_activity, slot_activity_bitset_first_inactive_bit_index);

            const int batch_group_tex_unit_index = zfw_get_sprite_batch_group_tex_unit_index(layer_index, i, tex_unit_index);
            batch_groups[batch_group_id].tex_units[batch_group_tex_unit_index].user_tex_index = user_tex_index;
            batch_groups[batch_group_id].tex_units[batch_group_tex_unit_index].count++;

            int slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_COUNT];
            slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_GROUP_INDEX] = batch_group_id;
            slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__LAYER_INDEX] = layer_index;
            slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_INDEX] = i;
            slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__SLOT_INDEX] = slot_activity_bitset_first_inactive_bit_index - slot_activity_bitset_begin_bit_index;
            slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__TEX_UNIT_INDEX] = tex_unit_index;

            return zfw_create_sprite_batch_slot_key(slot_key_elems);
        }
    }

    if (first_inactive_batch_index != -1)
    {
        // Initialise and activate a new sprite batch. If successful, try this all again.
        if (init_and_activate_render_layer_sprite_batch(layer_index, first_inactive_batch_index, &batch_groups[batch_group_id], main_mem_arena))
        {
            return zfw_take_render_layer_sprite_batch_slot(batch_group_id, layer_index, user_tex_index, batch_groups, main_mem_arena);
        }
    }

    // Return an inactive slot key.
    return 0;
}

void zfw_take_multiple_render_layer_sprite_batch_slots(zfw_sprite_batch_slot_key_t *const slot_keys, const int slot_key_count, const zfw_sprite_batch_group_id_t batch_group_id, const int layer_index, const int user_tex_index, zfw_sprite_batch_group_t batch_groups[ZFW_SPRITE_BATCH_GROUP_COUNT], zfw_mem_arena_t *const main_mem_arena)
{
    if (slot_key_count <= 0)
    {
        zfw_log_warning("Invalid \"slot_key_count\" parameter value (%d) for function \"zfw_take_multiple_slots_from_render_layer_sprite_batch\".", slot_key_count);
        return;
    }

    int slots_found_count = 0;
    int first_inactive_batch_index = -1;

    for (int i = 0; i < ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT; i++)
    {
        // Check if the batch is active.
        if (!(batch_groups[batch_group_id].batch_activity_bits[layer_index] & ((zfw_render_layer_sprite_batch_activity_bits_t)1 << i)))
        {
            if (first_inactive_batch_index == -1)
            {
                first_inactive_batch_index = i;
            }

            continue;
        }

        // Find a texture unit to use.
        int tex_unit_index = -1;

        for (int j = 0; j < get_gl_tex_unit_limit(); j++)
        {
            const zfw_sprite_batch_tex_unit_t tex_unit = batch_groups[batch_group_id].tex_units[zfw_get_sprite_batch_group_tex_unit_index(layer_index, i, j)];

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
        const int batch_group_tex_unit_index = zfw_get_sprite_batch_group_tex_unit_index(layer_index, i, tex_unit_index);

        for (int j = 0; j < ZFW_SPRITE_BATCH_SLOT_LIMIT; j++)
        {
            if (zfw_is_bitset_bit_active(&batch_groups[batch_group_id].slot_activity, slot_activity_bitset_begin_bit_index + j))
            {
                continue;
            }

            // Take the slot and add a key.
            zfw_activate_bitset_bit(&batch_groups[batch_group_id].slot_activity, slot_activity_bitset_begin_bit_index + j);

            batch_groups[batch_group_id].tex_units[batch_group_tex_unit_index].user_tex_index = user_tex_index;
            batch_groups[batch_group_id].tex_units[batch_group_tex_unit_index].count++;

            int slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_COUNT];
            slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_GROUP_INDEX] = batch_group_id;
            slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__LAYER_INDEX] = layer_index;
            slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_INDEX] = i;
            slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__SLOT_INDEX] = j;
            slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__TEX_UNIT_INDEX] = tex_unit_index;

            slot_keys[slots_found_count] = zfw_create_sprite_batch_slot_key(slot_key_elems);

            slots_found_count++;

            if (slots_found_count == slot_key_count)
            {
                return;
            }
        }
    }

    if (first_inactive_batch_index != -1)
    {
        // Initialise and activate a new sprite batch. If successful, try this all again.
        if (init_and_activate_render_layer_sprite_batch(layer_index, first_inactive_batch_index, &batch_groups[batch_group_id], main_mem_arena))
        {
            zfw_take_multiple_render_layer_sprite_batch_slots(slot_keys + slots_found_count, slot_key_count - slots_found_count, batch_group_id, layer_index, user_tex_index, batch_groups, main_mem_arena);
        }
    }
}

zfw_bool_t zfw_write_to_render_layer_sprite_batch_slot(const zfw_sprite_batch_slot_key_t slot_key, const zfw_vec_2d_t pos, const float rot, const zfw_vec_2d_t scale, const zfw_vec_2d_t origin, const zfw_rect_t *const src_rect, const zfw_color_t *const blend, const zfw_sprite_batch_group_t batch_groups[ZFW_SPRITE_BATCH_GROUP_COUNT], const zfw_user_tex_data_t *const user_tex_data)
{
    if (!zfw_is_sprite_batch_slot_key_active(slot_key))
    {
        zfw_log_warning("Attempting to write to a render layer sprite batch slot using an inactive key!");
        return ZFW_FALSE;
    }

    int slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_COUNT];
    zfw_init_sprite_batch_slot_key_elems(slot_key, slot_key_elems);

    const zfw_sprite_batch_group_t *const batch_group = &batch_groups[slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_GROUP_INDEX]];

    const int user_tex_index = batch_group->tex_units[zfw_get_sprite_batch_group_tex_unit_index(slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__LAYER_INDEX], slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_INDEX], slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__TEX_UNIT_INDEX])].user_tex_index;
    const zfw_vec_2d_i_t user_tex_size = user_tex_data->sizes[user_tex_index];

    const float verts[ZFW_BUILTIN_SPRITE_QUAD_SHADER_PROG_VERT_COUNT * 4] = {
        (0.0f - origin.x) * scale.x,
        (0.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        src_rect->w,
        src_rect->h,
        rot,
        slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__TEX_UNIT_INDEX],
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
        src_rect->w,
        src_rect->h,
        rot,
        slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__TEX_UNIT_INDEX],
        (float)(src_rect->x + src_rect->w) / user_tex_size.x,
        (float)src_rect->y / user_tex_size.y,
        blend->r,
        blend->g,
        blend->b,
        blend->a,

        (1.0f - origin.x) * scale.x,
        (1.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        src_rect->w,
        src_rect->h,
        rot,
        slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__TEX_UNIT_INDEX],
        (float)(src_rect->x + src_rect->w) / user_tex_size.x,
        (float)(src_rect->y + src_rect->h) / user_tex_size.y,
        blend->r,
        blend->g,
        blend->b,
        blend->a,

        (0.0f - origin.x) * scale.x,
        (1.0f - origin.y) * scale.y,
        pos.x,
        pos.y,
        src_rect->w,
        src_rect->h,
        rot,
        slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__TEX_UNIT_INDEX],
        (float)src_rect->x / user_tex_size.x,
        (float)(src_rect->y + src_rect->h) / user_tex_size.y,
        blend->r,
        blend->g,
        blend->b,
        blend->a
    };

    const int batch_group_batch_index = zfw_get_sprite_batch_group_batch_index(slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__LAYER_INDEX], slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_INDEX]);
    glBindVertexArray(batch_group->vert_array_gl_ids[batch_group_batch_index]);
    glBindBuffer(GL_ARRAY_BUFFER, batch_group->vert_buf_gl_ids[batch_group_batch_index]);
    glBufferSubData(GL_ARRAY_BUFFER, slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__SLOT_INDEX] * sizeof(verts), sizeof(verts), verts);

    return ZFW_TRUE;
}

zfw_bool_t zfw_clear_render_layer_sprite_batch_slot(const zfw_sprite_batch_slot_key_t slot_key, const zfw_sprite_batch_group_t batch_groups[ZFW_SPRITE_BATCH_GROUP_COUNT])
{
    if (!zfw_is_sprite_batch_slot_key_active(slot_key))
    {
        zfw_log_warning("Attempting to clear a render layer sprite batch slot using an inactive key!");
        return ZFW_FALSE;
    }

    int slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_COUNT];
    zfw_init_sprite_batch_slot_key_elems(slot_key, slot_key_elems);

    const zfw_sprite_batch_group_t *const batch_group = &batch_groups[slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_GROUP_INDEX]];
    const int batch_group_batch_index = zfw_get_sprite_batch_group_batch_index(slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__LAYER_INDEX], slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_INDEX]);

    glBindVertexArray(batch_group->vert_array_gl_ids[batch_group_batch_index]);
    glBindBuffer(GL_ARRAY_BUFFER, batch_group->vert_buf_gl_ids[batch_group_batch_index]);

    const float verts[ZFW_BUILTIN_SPRITE_QUAD_SHADER_PROG_VERT_COUNT * 4] = {0};
    glBufferSubData(GL_ARRAY_BUFFER, slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__SLOT_INDEX] * sizeof(verts), sizeof(verts), verts);

    return ZFW_TRUE;
}

zfw_bool_t zfw_free_render_layer_sprite_batch_slot(const zfw_sprite_batch_slot_key_t slot_key, zfw_sprite_batch_group_t batch_groups[ZFW_SPRITE_BATCH_GROUP_COUNT])
{
    if (!zfw_is_sprite_batch_slot_key_active(slot_key))
    {
        zfw_log_warning("Attempting to free a render layer sprite batch slot using an inactive key!");
        return ZFW_FALSE;
    }

    int slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_COUNT];
    zfw_init_sprite_batch_slot_key_elems(slot_key, slot_key_elems);

    zfw_sprite_batch_group_t *const batch_group = &batch_groups[slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_GROUP_INDEX]];
    const int batch_group_batch_index = zfw_get_sprite_batch_group_batch_index(slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__LAYER_INDEX], slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_INDEX]);

    glBindVertexArray(batch_group->vert_array_gl_ids[batch_group_batch_index]);
    glBindBuffer(GL_ARRAY_BUFFER, batch_group->vert_buf_gl_ids[batch_group_batch_index]);

    const float verts[ZFW_BUILTIN_SPRITE_QUAD_SHADER_PROG_VERT_COUNT * 4] = {0};
    glBufferSubData(GL_ARRAY_BUFFER, slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__SLOT_INDEX] * sizeof(verts), sizeof(verts), verts);

    zfw_deactivate_bitset_bit(&batch_group->slot_activity, zfw_get_sprite_batch_group_slot_index(slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__LAYER_INDEX], slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_INDEX], slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__SLOT_INDEX]));

    batch_group->tex_units[zfw_get_sprite_batch_group_tex_unit_index(slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__LAYER_INDEX], slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__BATCH_INDEX], slot_key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_ID__TEX_UNIT_INDEX])].count--;

    return ZFW_TRUE;
}

zfw_sprite_batch_slot_key_t zfw_create_sprite_batch_slot_key(const int key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_COUNT])
{
    zfw_sprite_batch_slot_key_t slot_key = 1; // Initialise the key as active.
    int bit_index = 1;

    for (int i = 0; i < ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_COUNT; i++)
    {
        slot_key |= key_elems[i] << bit_index;
        bit_index += get_sprite_batch_slot_key_elem_bit_count(i);
    }

    return slot_key;
}

int zfw_get_sprite_batch_slot_key_elem(const zfw_sprite_batch_slot_key_t slot_key, const zfw_sprite_batch_slot_key_elem_id_t elem_id)
{
    int bit_index = 1; // (Skip the activity bit.)

    for (int i = 0; i < elem_id; i++)
    {
        bit_index += get_sprite_batch_slot_key_elem_bit_count(i);
    }

    return (slot_key >> bit_index) & (get_sprite_batch_slot_key_elem_bit_count(elem_id) - 1);
}

void zfw_init_sprite_batch_slot_key_elems(const zfw_sprite_batch_slot_key_t slot_key, int key_elems[ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_COUNT])
{
    int bit_index = 1; // (Skip the activity bit.)

    for (int i = 0; i < ZFW_SPRITE_BATCH_SLOT_KEY_ELEM_COUNT; i++)
    {
        key_elems[i] = (slot_key >> bit_index) & ((1 << get_sprite_batch_slot_key_elem_bit_count(i)) - 1);
        bit_index += get_sprite_batch_slot_key_elem_bit_count(i);
    }
}

zfw_bool_t zfw_init_char_batch_group(zfw_char_batch_group_t *const batch_group, zfw_mem_arena_t *const main_mem_arena)
{
    memset(batch_group, 0, sizeof(*batch_group));

    const int batch_group_batch_count = ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT;

    // Generate vertex arrays after allocating memory for OpenGL IDs.
    batch_group->vert_array_gl_ids = zfw_mem_arena_alloc(main_mem_arena, sizeof(*batch_group->vert_array_gl_ids) * batch_group_batch_count);

    if (!batch_group->vert_array_gl_ids)
    {
        return ZFW_FALSE;
    }

    glGenVertexArrays(batch_group_batch_count, batch_group->vert_array_gl_ids);

    // Generate vertex buffers after allocating memory for OpenGL IDs.
    batch_group->vert_buf_gl_ids = zfw_mem_arena_alloc(main_mem_arena, sizeof(*batch_group->vert_buf_gl_ids) * batch_group_batch_count);

    if (!batch_group->vert_buf_gl_ids)
    {
        return ZFW_FALSE;
    }

    glGenBuffers(batch_group_batch_count, batch_group->vert_buf_gl_ids);

    // Generate element buffers after allocating memory for OpenGL IDs.
    batch_group->elem_buf_gl_ids = zfw_mem_arena_alloc(main_mem_arena, sizeof(*batch_group->elem_buf_gl_ids) * batch_group_batch_count);

    if (!batch_group->elem_buf_gl_ids)
    {
        return ZFW_FALSE;
    }

    glGenBuffers(batch_group_batch_count, batch_group->elem_buf_gl_ids);

    // Allocate memory for batch user font indexes.
    batch_group->user_font_indexes = zfw_mem_arena_alloc(main_mem_arena, sizeof(*batch_group->user_font_indexes) * batch_group_batch_count);

    if (!batch_group->user_font_indexes)
    {
        return ZFW_FALSE;
    }

    // Allocate memory for batch positions.
    batch_group->positions = zfw_mem_arena_alloc(main_mem_arena, sizeof(*batch_group->positions) * batch_group_batch_count);

    if (!batch_group->positions)
    {
        return ZFW_FALSE;
    }

    // Allocate memory for batch scales.
    batch_group->scales = zfw_mem_arena_alloc(main_mem_arena, sizeof(*batch_group->scales) * batch_group_batch_count);

    if (!batch_group->scales)
    {
        return ZFW_FALSE;
    }

    // Allocate memory for batch blends.
    batch_group->blends = zfw_mem_arena_alloc(main_mem_arena, sizeof(*batch_group->blends) * batch_group_batch_count);

    if (!batch_group->blends)
    {
        return ZFW_FALSE;
    }

    return ZFW_TRUE;
}

void zfw_clean_char_batch_group(zfw_char_batch_group_t *const batch_group)
{
    if (batch_group->elem_buf_gl_ids)
    {
        glDeleteBuffers(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, batch_group->elem_buf_gl_ids);
    }

    if (batch_group->vert_buf_gl_ids)
    {
        glDeleteBuffers(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, batch_group->vert_buf_gl_ids);
    }

    if (batch_group->vert_array_gl_ids)
    {
        glDeleteVertexArrays(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, batch_group->vert_array_gl_ids);
    }
}

void zfw_set_char_batch_group_defaults(zfw_char_batch_group_t *const batch_group)
{
    memset(batch_group->batch_init_bits, 0, sizeof(batch_group->batch_init_bits));
    memset(batch_group->batch_activity_bits, 0, sizeof(batch_group->batch_activity_bits));

    for (int i = 0; i < ZFW_RENDER_LAYER_LIMIT; i++)
    {
        for (int j = 0; j < ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT; j++)
        {
            const int batch_group_batch_index = zfw_get_char_batch_group_batch_index(i, j);

            batch_group->user_font_indexes[batch_group_batch_index] = 0;

            batch_group->positions[batch_group_batch_index] = zfw_create_vec_2d(0.0f, 0.0f);

            batch_group->scales[batch_group_batch_index].x = 1.0f;
            batch_group->scales[batch_group_batch_index].y = 1.0f;

            batch_group->blends[batch_group_batch_index].r = 1.0f;
            batch_group->blends[batch_group_batch_index].g = 1.0f;
            batch_group->blends[batch_group_batch_index].b = 1.0f;
            batch_group->blends[batch_group_batch_index].a = 1.0f;
        }
    }
}

zfw_char_batch_key_t zfw_take_render_layer_char_batch(const int layer_index, zfw_char_batch_group_t *const batch_group, zfw_mem_arena_t *const main_mem_arena)
{
    for (int i = 0; i < ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT; i++)
    {
        const zfw_render_layer_char_batch_bits_t batch_bitmask = (zfw_render_layer_char_batch_bits_t)1 << i;

        if (!(batch_group->batch_activity_bits[layer_index] & batch_bitmask))
        {
            // Initialise the batch if not already done.
            if (!(batch_group->batch_init_bits[layer_index] & batch_bitmask))
            {
                const int batch_group_batch_index = zfw_get_char_batch_group_batch_index(layer_index, i);

                glBindVertexArray(batch_group->vert_array_gl_ids[batch_group_batch_index]);

                glBindBuffer(GL_ARRAY_BUFFER, batch_group->vert_buf_gl_ids[batch_group_batch_index]);

                {
                    float *verts;
                    const int verts_size = sizeof(*verts) * ZFW_BUILTIN_CHAR_QUAD_SHADER_PROG_VERT_COUNT * 4 * ZFW_CHAR_BATCH_SLOT_LIMIT;
                    verts = zfw_mem_arena_alloc(main_mem_arena, verts_size);

                    if (!verts)
                    {
                        zfw_log_error("Failed to allocate %d bytes for render layer character batch vertices!", verts_size);
                        return 0;
                    }

                    memset(verts, 0, verts_size);

                    glBufferData(GL_ARRAY_BUFFER, verts_size, verts, GL_DYNAMIC_DRAW);

                    zfw_rewind_mem_arena(main_mem_arena);
                }

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch_group->elem_buf_gl_ids[batch_group_batch_index]);

                {
                    unsigned short *indices;
                    const int indices_size = sizeof(*indices) * 6 * ZFW_CHAR_BATCH_SLOT_LIMIT;
                    indices = zfw_mem_arena_alloc(main_mem_arena, indices_size);

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

                    zfw_rewind_mem_arena(main_mem_arena);
                }

                const int verts_stride = sizeof(float) * ZFW_BUILTIN_CHAR_QUAD_SHADER_PROG_VERT_COUNT;

                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, verts_stride, (void *)(sizeof(float) * 0));
                glEnableVertexAttribArray(0);

                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, verts_stride, (void *)(sizeof(float) * 2));
                glEnableVertexAttribArray(1);

                glBindVertexArray(0);

                batch_group->batch_init_bits[layer_index] |= batch_bitmask;
            }

            // Take the batch and return a key.
            batch_group->batch_activity_bits[layer_index] |= batch_bitmask;

            return zfw_create_char_batch_key(layer_index, i);
        }
    }

    return 0;
}

zfw_bool_t zfw_write_to_render_layer_char_batch(const zfw_char_batch_key_t key, const char *const text, const zfw_font_hor_align_t hor_align, const zfw_font_vert_align_t vert_align, zfw_char_batch_group_t *const batch_group, const zfw_user_font_data_t *const user_font_data)
{
    if (!zfw_is_char_batch_slot_key_active(key))
    {
        zfw_log_warning("Attempting to write to a render layer character batch using an inactive key!");
        return ZFW_FALSE;
    }

    const int batch_group_batch_index = zfw_get_char_batch_group_batch_index(zfw_get_char_batch_slot_key_layer_index(key), zfw_get_char_batch_slot_key_batch_index(key));

    const int batch_user_font_index = batch_group->user_font_indexes[batch_group_batch_index];

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

    int text_line_ws[ZFW_CHAR_BATCH_SLOT_LIMIT] = {0};
    int text_first_line_min_offs;
    int text_first_line_min_offs_updated = ZFW_FALSE;
    int text_last_line_max_h;
    int text_last_line_max_h_updated = ZFW_FALSE;
    int text_line_counter = 0;

    for (int i = 0; i < text_len; i++)
    {
        if (text[i] == '\n')
        {
            text_line_ws[text_line_counter] = char_draw_pos_pen.x;

            if (!text_first_line_min_offs_updated)
            {
                // Set the first line minimum offset to the vertical offset of the space character.
                text_first_line_min_offs = user_font_data->chars_vert_offsets[batch_user_font_index * ZFW_FONT_CHAR_RANGE_SIZE];
                text_first_line_min_offs_updated = ZFW_TRUE;
            }

            // Set the last line maximum h to the h of a space.
            text_last_line_max_h = user_font_data->chars_vert_offsets[batch_user_font_index * ZFW_FONT_CHAR_RANGE_SIZE] + user_font_data->chars_src_rects[batch_user_font_index * ZFW_FONT_CHAR_RANGE_SIZE].h;

            text_last_line_max_h_updated = ZFW_FALSE;

            text_line_counter++;

            // Move the pen to a new line.
            char_draw_pos_pen.x = 0.0f;
            char_draw_pos_pen.y += user_font_data->line_heights[batch_user_font_index];

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

        if (!text_last_line_max_h_updated)
        {
            text_last_line_max_h = user_font_data->chars_vert_offsets[user_font_chars_index] + user_font_data->chars_src_rects[user_font_chars_index].h;
            text_last_line_max_h_updated = ZFW_TRUE;
        }
        else
        {
            text_last_line_max_h = ZFW_MAX(user_font_data->chars_vert_offsets[user_font_chars_index] + user_font_data->chars_src_rects[user_font_chars_index].h, text_last_line_max_h);
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

    text_line_ws[text_line_counter] = char_draw_pos_pen.x;
    text_line_counter = 0;

    const int text_h = text_first_line_min_offs + char_draw_pos_pen.y + text_last_line_max_h;

    // Clear the batch then write the character render data.
    glBindVertexArray(batch_group->vert_array_gl_ids[batch_group_batch_index]);
    glBindBuffer(GL_ARRAY_BUFFER, batch_group->vert_buf_gl_ids[batch_group_batch_index]);

    {
        const float batch_clear_verts[(ZFW_BUILTIN_CHAR_QUAD_SHADER_PROG_VERT_COUNT * 4) * ZFW_CHAR_BATCH_SLOT_LIMIT] = {0};
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

            const zfw_vec_2d_t char_draw_pos = zfw_create_vec_2d(char_draw_positions[i].x - (text_line_ws[text_line_counter] * (hor_align / 2.0f)), char_draw_positions[i].y - (text_h * (vert_align / 2.0f)));

            zfw_rect_f_t char_tex_coords_rect;
            char_tex_coords_rect.x = (float)user_font_data->chars_src_rects[font_chars_index].x / user_font_data->tex_sizes[batch_user_font_index].x;
            char_tex_coords_rect.y = (float)user_font_data->chars_src_rects[font_chars_index].y / user_font_data->tex_sizes[batch_user_font_index].y;
            char_tex_coords_rect.w = (float)user_font_data->chars_src_rects[font_chars_index].w / user_font_data->tex_sizes[batch_user_font_index].x;
            char_tex_coords_rect.h = (float)user_font_data->chars_src_rects[font_chars_index].h / user_font_data->tex_sizes[batch_user_font_index].y;

            const float verts[ZFW_BUILTIN_CHAR_QUAD_SHADER_PROG_VERT_COUNT * 4] = {
                char_draw_pos.x,
                char_draw_pos.y,
                char_tex_coords_rect.x,
                char_tex_coords_rect.y,

                char_draw_pos.x + user_font_data->chars_src_rects[font_chars_index].w,
                char_draw_pos.y,
                char_tex_coords_rect.x + char_tex_coords_rect.w,
                char_tex_coords_rect.y,

                char_draw_pos.x + user_font_data->chars_src_rects[font_chars_index].w,
                char_draw_pos.y + user_font_data->chars_src_rects[font_chars_index].h,
                char_tex_coords_rect.x + char_tex_coords_rect.w,
                char_tex_coords_rect.y + char_tex_coords_rect.h,

                char_draw_pos.x,
                char_draw_pos.y + user_font_data->chars_src_rects[font_chars_index].h,
                char_tex_coords_rect.x,
                char_tex_coords_rect.y + char_tex_coords_rect.h
            };

            glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(verts), sizeof(verts), verts);
        }
    }

    return ZFW_TRUE;
}

zfw_bool_t zfw_clear_render_layer_char_batch(const zfw_char_batch_key_t key, zfw_char_batch_group_t *const batch_group)
{
    if (!zfw_is_char_batch_slot_key_active(key))
    {
        zfw_log_warning("Attempting to clear a render layer character batch using an inactive key!");
        return ZFW_FALSE;
    }

    const int batch_group_batch_index = zfw_get_char_batch_group_batch_index(zfw_get_char_batch_slot_key_layer_index(key), zfw_get_char_batch_slot_key_batch_index(key));

    glBindVertexArray(batch_group->vert_array_gl_ids[batch_group_batch_index]);
    glBindBuffer(GL_ARRAY_BUFFER, batch_group->vert_buf_gl_ids[batch_group_batch_index]);

    const float verts[(ZFW_BUILTIN_CHAR_QUAD_SHADER_PROG_VERT_COUNT * 4) * ZFW_CHAR_BATCH_SLOT_LIMIT] = {0};
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    return ZFW_TRUE;
}

zfw_bool_t zfw_free_render_layer_char_batch(const zfw_char_batch_key_t key, zfw_char_batch_group_t *const batch_group)
{
    if (!zfw_is_char_batch_slot_key_active(key))
    {
        zfw_log_warning("Attempting to free a render layer character batch using an inactive key!");
        return ZFW_FALSE;
    }

    const int layer_index = zfw_get_char_batch_slot_key_layer_index(key);
    const int batch_index = zfw_get_char_batch_slot_key_batch_index(key);
    const int batch_group_batch_index = zfw_get_char_batch_group_batch_index(layer_index, batch_index);

    glBindVertexArray(batch_group->vert_array_gl_ids[batch_group_batch_index]);
    glBindBuffer(GL_ARRAY_BUFFER, batch_group->vert_buf_gl_ids[batch_group_batch_index]);

    const float verts[(ZFW_BUILTIN_CHAR_QUAD_SHADER_PROG_VERT_COUNT * 4) * ZFW_CHAR_BATCH_SLOT_LIMIT] = {0};
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    batch_group->batch_activity_bits[layer_index] &= ~((zfw_render_layer_char_batch_bits_t)1 << batch_index);

    return ZFW_TRUE;
}

zfw_char_batch_key_t zfw_create_char_batch_key(const int layer_index, const int batch_index)
{
    zfw_char_batch_key_t key = 1; // Initialise the key as active.
    key |= (zfw_char_batch_key_t)layer_index << 1;
    key |= (zfw_char_batch_key_t)batch_index << (1 + (int)log2(ZFW_RENDER_LAYER_LIMIT));
    return key;
}

void zfw_render_sprite_and_character_batches(const zfw_sprite_batch_group_t sprite_batch_groups[ZFW_SPRITE_BATCH_GROUP_COUNT], const zfw_char_batch_group_t *const char_batch_group, const zfw_view_state_t *const view_state, const zfw_vec_2d_i_t window_size, const zfw_user_tex_data_t *const user_tex_data, const zfw_user_font_data_t *const user_font_data, const zfw_builtin_shader_prog_data_t *const builtin_shader_prog_data)
{
    zfw_matrix_4x4_t proj;
    zfw_init_ortho_matrix_4x4(&proj, 0.0f, window_size.x, window_size.y, 0.0f, -1.0f, 1.0f);

    // Draw view sprite batches.
    {
        glUseProgram(builtin_shader_prog_data->sprite_quad_prog_gl_id);

        zfw_matrix_4x4_t view;
        zfw_init_identity_matrix_4x4(&view);
        view.elems[0][0] = view_state->scale;
        view.elems[1][1] = view_state->scale;
        view.elems[2][2] = 1.0f;
        view.elems[3][0] = -view_state->pos.x * view_state->scale;
        view.elems[3][1] = -view_state->pos.y * view_state->scale;
        glUniformMatrix4fv(glGetUniformLocation(builtin_shader_prog_data->sprite_quad_prog_gl_id, "u_view"), 1, GL_FALSE, (float *)view.elems);

        glUniformMatrix4fv(glGetUniformLocation(builtin_shader_prog_data->sprite_quad_prog_gl_id, "u_proj"), 1, GL_FALSE, (float *)proj.elems);

        for (int i = 0; i < ZFW_RENDER_LAYER_LIMIT; i++)
        {
            draw_sprite_batches_of_layer(&sprite_batch_groups[ZFW_SPRITE_BATCH_GROUP_ID__VIEW], i, user_tex_data, builtin_shader_prog_data);
        }
    }

    // Draw screen sprite batches and character batches.
    for (int i = 0; i < ZFW_RENDER_LAYER_LIMIT; i++)
    {
        // Draw layer screen sprite batches.
        glUseProgram(builtin_shader_prog_data->sprite_quad_prog_gl_id);

        zfw_matrix_4x4_t view;
        zfw_init_identity_matrix_4x4(&view);
        glUniformMatrix4fv(glGetUniformLocation(builtin_shader_prog_data->sprite_quad_prog_gl_id, "u_view"), 1, GL_FALSE, (float *)view.elems);

        glUniformMatrix4fv(glGetUniformLocation(builtin_shader_prog_data->sprite_quad_prog_gl_id, "u_proj"), 1, GL_FALSE, (float *)proj.elems);

        draw_sprite_batches_of_layer(&sprite_batch_groups[ZFW_SPRITE_BATCH_GROUP_ID__SCREEN], i, user_tex_data, builtin_shader_prog_data);

        // Draw layer character batches.
        glUseProgram(builtin_shader_prog_data->char_quad_prog_gl_id);
        glUniformMatrix4fv(glGetUniformLocation(builtin_shader_prog_data->char_quad_prog_gl_id, "u_proj"), 1, GL_FALSE, (float *)proj.elems);
        draw_char_batches_of_layer(char_batch_group, i, user_font_data, builtin_shader_prog_data);
    }
}

void zfw_set_view_state_defaults(zfw_view_state_t *const view_state)
{
    view_state->pos = zfw_create_vec_2d(0.0f, 0.0f);
    view_state->scale = 1.0f;
}
