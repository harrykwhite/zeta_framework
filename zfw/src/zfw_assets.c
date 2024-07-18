#include <zfw.h>

#include <stdlib.h>

zfw_bool_t zfw_retrieve_user_asset_data_from_assets_file(zfw_user_asset_data_t *const user_asset_data, FILE *const assets_file_fs, zfw_mem_arena_t *const mem_arena)
{
    // Retrieve texture data.
    fread(&user_asset_data->tex_data.tex_count, sizeof(user_asset_data->tex_data.tex_count), 1, assets_file_fs);

    if (user_asset_data->tex_data.tex_count)
    {
        user_asset_data->tex_data.gl_ids = zfw_mem_arena_alloc(mem_arena, sizeof(*user_asset_data->tex_data.gl_ids) * user_asset_data->tex_data.tex_count);

        if (!user_asset_data->tex_data.gl_ids)
        {
            zfw_log_error("Failed to allocate %d bytes for texture OpenGL IDs!", sizeof(*user_asset_data->tex_data.gl_ids) * user_asset_data->tex_data.tex_count);
            return ZFW_FALSE;
        }

        glGenTextures(user_asset_data->tex_data.tex_count, user_asset_data->tex_data.gl_ids);

        user_asset_data->tex_data.sizes = zfw_mem_arena_alloc(mem_arena, sizeof(*user_asset_data->tex_data.sizes) * user_asset_data->tex_data.tex_count);

        if (!user_asset_data->tex_data.sizes)
        {
            zfw_log_error("Failed to allocate %d bytes for texture sizes!", sizeof(*user_asset_data->tex_data.sizes) * user_asset_data->tex_data.tex_count);
            return ZFW_FALSE;
        }

        for (int i = 0; i < user_asset_data->tex_data.tex_count; i++)
        {
            fread(&user_asset_data->tex_data.sizes[i], sizeof(user_asset_data->tex_data.sizes[i]), 1, assets_file_fs);

            const int px_data_size = user_asset_data->tex_data.sizes[i].x * user_asset_data->tex_data.sizes[i].y * ZFW_TEX_CHANNEL_COUNT;
            unsigned char *const px_data = zfw_mem_arena_alloc(mem_arena, px_data_size);

            if (!px_data)
            {
                zfw_log_error("Failed to allocate %d bytes for pixel data of user texture with index %d.", px_data_size, i);
                return ZFW_FALSE;
            }

            fread(px_data, sizeof(*px_data), px_data_size / sizeof(*px_data), assets_file_fs);

            glBindTexture(GL_TEXTURE_2D, user_asset_data->tex_data.gl_ids[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, user_asset_data->tex_data.sizes[i].x, user_asset_data->tex_data.sizes[i].y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data);

            // The pixel data for this texture is no longer needed in the arena, so rewind and allow it to be overwritten.
            zfw_rewind_mem_arena(mem_arena);
        }
    }

    // Retrieve shader program data.
    fread(&user_asset_data->shader_prog_data.prog_count, sizeof(user_asset_data->shader_prog_data.prog_count), 1, assets_file_fs);

    if (user_asset_data->shader_prog_data.prog_count)
    {
        user_asset_data->shader_prog_data.gl_ids = zfw_mem_arena_alloc(mem_arena, sizeof(*user_asset_data->shader_prog_data.gl_ids) * user_asset_data->shader_prog_data.prog_count);

        if (!user_asset_data->shader_prog_data.gl_ids)
        {
            zfw_log_error("Failed to allocate %d bytes for shader program OpenGL IDs!", sizeof(*user_asset_data->shader_prog_data.gl_ids) * user_asset_data->shader_prog_data.prog_count);
            return ZFW_FALSE;
        }

        for (int i = 0; i < user_asset_data->shader_prog_data.prog_count; i++)
        {
            char vert_shader_src_buf[ZFW_SHADER_SRC_BUF_SIZE];
            fread(vert_shader_src_buf, sizeof(*vert_shader_src_buf), sizeof(vert_shader_src_buf) / sizeof(*vert_shader_src_buf), assets_file_fs);

            char frag_shader_src_buf[ZFW_SHADER_SRC_BUF_SIZE];
            fread(frag_shader_src_buf, sizeof(*frag_shader_src_buf), sizeof(frag_shader_src_buf) / sizeof(*frag_shader_src_buf), assets_file_fs);

            zfw_gen_shader_prog(&user_asset_data->shader_prog_data.gl_ids[i], vert_shader_src_buf, frag_shader_src_buf);
        }
    }

    // Retrieve font data.
    fread(&user_asset_data->font_data.font_count, sizeof(user_asset_data->font_data.font_count), 1, assets_file_fs);

    if (user_asset_data->font_data.font_count)
    {
        // Allocate memory for and retrieve font line heights.
        {
            const int line_height_count = user_asset_data->font_data.font_count;
            user_asset_data->font_data.line_heights = zfw_mem_arena_alloc(mem_arena, sizeof(*user_asset_data->font_data.line_heights) * line_height_count);

            if (!user_asset_data->font_data.line_heights)
            {
                zfw_log_error("Failed to allocate %d bytes for font line heights!", sizeof(*user_asset_data->font_data.line_heights) * line_height_count);
                return ZFW_FALSE;
            }

            fread(user_asset_data->font_data.line_heights, sizeof(*user_asset_data->font_data.line_heights), line_height_count, assets_file_fs);
        }

        // Allocate memory for and retrieve font character horizontal offsets.
        {
            const int hor_offs_count = ZFW_FONT_CHAR_RANGE_SIZE * user_asset_data->font_data.font_count;
            user_asset_data->font_data.chars_hor_offsets = zfw_mem_arena_alloc(mem_arena, sizeof(*user_asset_data->font_data.chars_hor_offsets) * hor_offs_count);

            if (!user_asset_data->font_data.chars_hor_offsets)
            {
                zfw_log_error("Failed to allocate %d bytes for font character horizontal offsets!", sizeof(*user_asset_data->font_data.chars_hor_offsets) * hor_offs_count);
                return ZFW_FALSE;
            }

            fread(user_asset_data->font_data.chars_hor_offsets, sizeof(*user_asset_data->font_data.chars_hor_offsets), hor_offs_count, assets_file_fs);
        }

        // Allocate memory for and retrieve font character vertical offsets.
        {
            const int vert_offs_count = ZFW_FONT_CHAR_RANGE_SIZE * user_asset_data->font_data.font_count;
            user_asset_data->font_data.chars_vert_offsets = zfw_mem_arena_alloc(mem_arena, sizeof(*user_asset_data->font_data.chars_vert_offsets) * vert_offs_count);

            if (!user_asset_data->font_data.chars_vert_offsets)
            {
                zfw_log_error("Failed to allocate %d bytes for font character vertical offsets!", sizeof(*user_asset_data->font_data.chars_vert_offsets) * vert_offs_count);
                return ZFW_FALSE;
            }

            fread(user_asset_data->font_data.chars_vert_offsets, sizeof(*user_asset_data->font_data.chars_vert_offsets), vert_offs_count, assets_file_fs);
        }

        // Allocate memory for and retrieve font character horizontal advances.
        {
            const int hor_advance_count = ZFW_FONT_CHAR_RANGE_SIZE * user_asset_data->font_data.font_count;
            user_asset_data->font_data.chars_hor_advances = zfw_mem_arena_alloc(mem_arena, sizeof(*user_asset_data->font_data.chars_hor_advances) * hor_advance_count);

            if (!user_asset_data->font_data.chars_hor_advances)
            {
                zfw_log_error("Failed to allocate %d bytes for font character horizontal advances!", sizeof(*user_asset_data->font_data.chars_hor_advances) * hor_advance_count);
                return ZFW_FALSE;
            }

            fread(user_asset_data->font_data.chars_hor_advances, sizeof(*user_asset_data->font_data.chars_hor_advances), hor_advance_count, assets_file_fs);
        }

        // Allocate memory for and retrieve font character source rectangles.
        {
            const int src_rect_count = ZFW_FONT_CHAR_RANGE_SIZE * user_asset_data->font_data.font_count;
            user_asset_data->font_data.chars_src_rects = zfw_mem_arena_alloc(mem_arena, sizeof(*user_asset_data->font_data.chars_src_rects) * src_rect_count);

            if (!user_asset_data->font_data.chars_src_rects)
            {
                zfw_log_error("Failed to allocate %d bytes for font character source rectangles!", sizeof(*user_asset_data->font_data.chars_src_rects) * src_rect_count);
                return ZFW_FALSE;
            }

            fread(user_asset_data->font_data.chars_src_rects, sizeof(*user_asset_data->font_data.chars_src_rects), src_rect_count, assets_file_fs);
        }

        // Allocate memory for and retrieve font character kernings.
        {
            const int kerning_count = ZFW_FONT_CHAR_RANGE_SIZE * ZFW_FONT_CHAR_RANGE_SIZE * user_asset_data->font_data.font_count;
            user_asset_data->font_data.chars_kernings = zfw_mem_arena_alloc(mem_arena, sizeof(*user_asset_data->font_data.chars_kernings) * kerning_count);

            if (!user_asset_data->font_data.chars_kernings)
            {
                zfw_log_error("Failed to allocate %d bytes for font character kernings!", sizeof(*user_asset_data->font_data.chars_kernings) * kerning_count);
                return ZFW_FALSE;
            }

            fread(user_asset_data->font_data.chars_kernings, sizeof(*user_asset_data->font_data.chars_kernings), kerning_count, assets_file_fs);
        }

        // Allocate memory for and retrieve font texture sizes.
        {
            const int tex_size_count = user_asset_data->font_data.font_count;
            user_asset_data->font_data.tex_sizes = zfw_mem_arena_alloc(mem_arena, sizeof(*user_asset_data->font_data.tex_sizes) * tex_size_count);

            if (!user_asset_data->font_data.tex_sizes)
            {
                zfw_log_error("Failed to allocate %d bytes for font texture sizes!", sizeof(*user_asset_data->font_data.tex_sizes) * tex_size_count);
                return ZFW_FALSE;
            }

            fread(user_asset_data->font_data.tex_sizes, sizeof(*user_asset_data->font_data.tex_sizes), tex_size_count, assets_file_fs);
        }

        // Allocate memory for OpenGL texture IDs and generate the textures.
        user_asset_data->font_data.tex_glids = zfw_mem_arena_alloc(mem_arena, sizeof(*user_asset_data->font_data.tex_glids) * user_asset_data->font_data.font_count);

        if (!user_asset_data->font_data.tex_glids)
        {
            zfw_log_error("Failed to allocate %d bytes for font texture OpenGL IDs!", sizeof(*user_asset_data->font_data.tex_glids) * user_asset_data->font_data.font_count);
            return ZFW_FALSE;
        }

        glGenTextures(user_asset_data->font_data.font_count, user_asset_data->font_data.tex_glids);

        // Finish generating the font textures using pixel data in the file.
        for (int i = 0; i < user_asset_data->font_data.font_count; i++)
        {
            const int px_data_size = user_asset_data->font_data.tex_sizes[i].x * user_asset_data->font_data.tex_sizes[i].y * ZFW_FONT_TEX_CHANNEL_COUNT;
            unsigned char *const px_data = zfw_mem_arena_alloc(mem_arena, px_data_size);

            if (!px_data)
            {
                zfw_log_error("Failed to allocate %d bytes for font texture pixel data of font with index %d.", px_data_size, i);
                return ZFW_FALSE;
            }

            fread(px_data, sizeof(*px_data), px_data_size / sizeof(*px_data), assets_file_fs);

            glBindTexture(GL_TEXTURE_2D, user_asset_data->font_data.tex_glids[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, user_asset_data->font_data.tex_sizes[i].x, user_asset_data->font_data.tex_sizes[i].y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data);

            // The pixel data for this font texture is no longer needed in the arena, so rewind and allow it to be overwritten.
            zfw_rewind_mem_arena(mem_arena);
        }
    }
    //

    return ZFW_TRUE;
}

void zfw_gen_shader_prog(GLuint *const shader_prog_gl_id, const char *const vert_shader_src, const char *const frag_shader_src)
{
    // Create the vertex shader.
    const GLuint vert_shader_gl_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader_gl_id, 1, &vert_shader_src, NULL);

    glCompileShader(vert_shader_gl_id);

    // Create the fragment shader.
    const GLuint frag_shader_gl_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader_gl_id, 1, &frag_shader_src, NULL);

    glCompileShader(frag_shader_gl_id);

    // Create the shader program using the shaders.
    *shader_prog_gl_id = glCreateProgram();

    glAttachShader(*shader_prog_gl_id, vert_shader_gl_id);
    glAttachShader(*shader_prog_gl_id, frag_shader_gl_id);

    glLinkProgram(*shader_prog_gl_id);

    // Delete the shaders as they're no longer needed.
    glDeleteShader(frag_shader_gl_id);
    glDeleteShader(vert_shader_gl_id);
}
