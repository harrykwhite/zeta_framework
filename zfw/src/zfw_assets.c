#include <zfw_assets.h>

#include <zfw_common_debug.h>

void zfw_gen_shader_prog(GLuint *const shader_prog_gl_id, const char *const vert_shader_src,
                         const char *const frag_shader_src)
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

zfw_bool_t zfw_retrieve_user_asset_data_from_assets_file(zfw_user_tex_data_t *const tex_data,
                                                         zfw_user_shader_prog_data_t *const shader_prog_data,
                                                         zfw_user_font_data_t *const font_data,
                                                         FILE *const assets_file_fs,
                                                         zfw_mem_arena_t *const main_mem_arena)
{
    // Retrieve texture data.
    fread(&tex_data->tex_count, sizeof(tex_data->tex_count), 1, assets_file_fs);

    if (tex_data->tex_count)
    {
        tex_data->gl_ids = zfw_mem_arena_alloc(main_mem_arena, sizeof(*tex_data->gl_ids) * tex_data->tex_count);

        if (!tex_data->gl_ids)
        {
            zfw_log_error("Failed to allocate %d bytes for texture OpenGL IDs!",
                          sizeof(*tex_data->gl_ids) * tex_data->tex_count);
            return ZFW_FALSE;
        }

        glGenTextures(tex_data->tex_count, tex_data->gl_ids);

        tex_data->sizes = zfw_mem_arena_alloc(main_mem_arena, sizeof(*tex_data->sizes) * tex_data->tex_count);

        if (!tex_data->sizes)
        {
            zfw_log_error("Failed to allocate %d bytes for texture sizes!",
                          sizeof(*tex_data->sizes) * tex_data->tex_count);
            return ZFW_FALSE;
        }

        for (int i = 0; i < tex_data->tex_count; i++)
        {
            fread(&tex_data->sizes[i], sizeof(tex_data->sizes[i]), 1, assets_file_fs);

            const int px_data_size = tex_data->sizes[i].x * tex_data->sizes[i].y * ZFW_TEX_CHANNEL_COUNT;
            unsigned char *const px_data = zfw_mem_arena_alloc(main_mem_arena, px_data_size);

            if (!px_data)
            {
                zfw_log_error("Failed to allocate %d bytes for pixel data of user texture with index %d.", px_data_size,
                              i);
                return ZFW_FALSE;
            }

            fread(px_data, sizeof(*px_data), px_data_size / sizeof(*px_data), assets_file_fs);

            glBindTexture(GL_TEXTURE_2D, tex_data->gl_ids[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_data->sizes[i].x, tex_data->sizes[i].y, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, px_data);

            // The pixel data for this texture is no longer needed in the arena, so rewind and allow it to be
            // overwritten.
            zfw_rewind_mem_arena(main_mem_arena);
        }
    }

    // Retrieve shader program data.
    fread(&shader_prog_data->prog_count, sizeof(shader_prog_data->prog_count), 1, assets_file_fs);

    if (shader_prog_data->prog_count)
    {
        shader_prog_data->gl_ids =
            zfw_mem_arena_alloc(main_mem_arena, sizeof(*shader_prog_data->gl_ids) * shader_prog_data->prog_count);

        if (!shader_prog_data->gl_ids)
        {
            zfw_log_error("Failed to allocate %d bytes for shader program OpenGL IDs!",
                          sizeof(*shader_prog_data->gl_ids) * shader_prog_data->prog_count);
            return ZFW_FALSE;
        }

        for (int i = 0; i < shader_prog_data->prog_count; i++)
        {
            char vert_shader_src_buf[ZFW_SHADER_SRC_BUF_SIZE];
            fread(vert_shader_src_buf, sizeof(*vert_shader_src_buf),
                  sizeof(vert_shader_src_buf) / sizeof(*vert_shader_src_buf), assets_file_fs);

            char frag_shader_src_buf[ZFW_SHADER_SRC_BUF_SIZE];
            fread(frag_shader_src_buf, sizeof(*frag_shader_src_buf),
                  sizeof(frag_shader_src_buf) / sizeof(*frag_shader_src_buf), assets_file_fs);

            zfw_gen_shader_prog(&shader_prog_data->gl_ids[i], vert_shader_src_buf, frag_shader_src_buf);
        }
    }

    // Retrieve font data.
    fread(&font_data->font_count, sizeof(font_data->font_count), 1, assets_file_fs);

    if (font_data->font_count)
    {
        // Allocate memory for and retrieve font line heights.
        {
            const int line_height_count = font_data->font_count;
            font_data->line_heights =
                zfw_mem_arena_alloc(main_mem_arena, sizeof(*font_data->line_heights) * line_height_count);

            if (!font_data->line_heights)
            {
                zfw_log_error("Failed to allocate %d bytes for font line heights!",
                              sizeof(*font_data->line_heights) * line_height_count);
                return ZFW_FALSE;
            }

            fread(font_data->line_heights, sizeof(*font_data->line_heights), line_height_count, assets_file_fs);
        }

        // Allocate memory for and retrieve font character horizontal offsets.
        {
            const int hor_offs_count = ZFW_FONT_CHAR_RANGE_SIZE * font_data->font_count;
            font_data->chars_hor_offsets =
                zfw_mem_arena_alloc(main_mem_arena, sizeof(*font_data->chars_hor_offsets) * hor_offs_count);

            if (!font_data->chars_hor_offsets)
            {
                zfw_log_error("Failed to allocate %d bytes for font character horizontal offsets!",
                              sizeof(*font_data->chars_hor_offsets) * hor_offs_count);
                return ZFW_FALSE;
            }

            fread(font_data->chars_hor_offsets, sizeof(*font_data->chars_hor_offsets), hor_offs_count, assets_file_fs);
        }

        // Allocate memory for and retrieve font character vertical offsets.
        {
            const int vert_offs_count = ZFW_FONT_CHAR_RANGE_SIZE * font_data->font_count;
            font_data->chars_vert_offsets =
                zfw_mem_arena_alloc(main_mem_arena, sizeof(*font_data->chars_vert_offsets) * vert_offs_count);

            if (!font_data->chars_vert_offsets)
            {
                zfw_log_error("Failed to allocate %d bytes for font character vertical offsets!",
                              sizeof(*font_data->chars_vert_offsets) * vert_offs_count);
                return ZFW_FALSE;
            }

            fread(font_data->chars_vert_offsets, sizeof(*font_data->chars_vert_offsets), vert_offs_count,
                  assets_file_fs);
        }

        // Allocate memory for and retrieve font character horizontal advances.
        {
            const int hor_advance_count = ZFW_FONT_CHAR_RANGE_SIZE * font_data->font_count;
            font_data->chars_hor_advances =
                zfw_mem_arena_alloc(main_mem_arena, sizeof(*font_data->chars_hor_advances) * hor_advance_count);

            if (!font_data->chars_hor_advances)
            {
                zfw_log_error("Failed to allocate %d bytes for font character horizontal advances!",
                              sizeof(*font_data->chars_hor_advances) * hor_advance_count);
                return ZFW_FALSE;
            }

            fread(font_data->chars_hor_advances, sizeof(*font_data->chars_hor_advances), hor_advance_count,
                  assets_file_fs);
        }

        // Allocate memory for and retrieve font character source rectangles.
        {
            const int src_rect_count = ZFW_FONT_CHAR_RANGE_SIZE * font_data->font_count;
            font_data->chars_src_rects =
                zfw_mem_arena_alloc(main_mem_arena, sizeof(*font_data->chars_src_rects) * src_rect_count);

            if (!font_data->chars_src_rects)
            {
                zfw_log_error("Failed to allocate %d bytes for font character source rectangles!",
                              sizeof(*font_data->chars_src_rects) * src_rect_count);
                return ZFW_FALSE;
            }

            fread(font_data->chars_src_rects, sizeof(*font_data->chars_src_rects), src_rect_count, assets_file_fs);
        }

        // Allocate memory for and retrieve font character kernings.
        {
            const int kerning_count = ZFW_FONT_CHAR_RANGE_SIZE * ZFW_FONT_CHAR_RANGE_SIZE * font_data->font_count;
            font_data->chars_kernings =
                zfw_mem_arena_alloc(main_mem_arena, sizeof(*font_data->chars_kernings) * kerning_count);

            if (!font_data->chars_kernings)
            {
                zfw_log_error("Failed to allocate %d bytes for font character kernings!",
                              sizeof(*font_data->chars_kernings) * kerning_count);
                return ZFW_FALSE;
            }

            fread(font_data->chars_kernings, sizeof(*font_data->chars_kernings), kerning_count, assets_file_fs);
        }

        // Allocate memory for and retrieve font texture sizes.
        {
            const int tex_size_count = font_data->font_count;
            font_data->tex_sizes = zfw_mem_arena_alloc(main_mem_arena, sizeof(*font_data->tex_sizes) * tex_size_count);

            if (!font_data->tex_sizes)
            {
                zfw_log_error("Failed to allocate %d bytes for font texture sizes!",
                              sizeof(*font_data->tex_sizes) * tex_size_count);
                return ZFW_FALSE;
            }

            fread(font_data->tex_sizes, sizeof(*font_data->tex_sizes), tex_size_count, assets_file_fs);
        }

        // Allocate memory for OpenGL texture IDs and generate the textures.
        font_data->tex_gl_ids =
            zfw_mem_arena_alloc(main_mem_arena, sizeof(*font_data->tex_gl_ids) * font_data->font_count);

        if (!font_data->tex_gl_ids)
        {
            zfw_log_error("Failed to allocate %d bytes for font texture OpenGL IDs!",
                          sizeof(*font_data->tex_gl_ids) * font_data->font_count);
            return ZFW_FALSE;
        }

        glGenTextures(font_data->font_count, font_data->tex_gl_ids);

        // Finish generating the font textures using pixel data in the file.
        for (int i = 0; i < font_data->font_count; i++)
        {
            const int px_data_size = font_data->tex_sizes[i].x * font_data->tex_sizes[i].y * ZFW_FONT_TEX_CHANNEL_COUNT;
            unsigned char *const px_data = zfw_mem_arena_alloc(main_mem_arena, px_data_size);

            if (!px_data)
            {
                zfw_log_error("Failed to allocate %d bytes for font texture pixel data of font with index %d.",
                              px_data_size, i);
                return ZFW_FALSE;
            }

            fread(px_data, sizeof(*px_data), px_data_size / sizeof(*px_data), assets_file_fs);

            glBindTexture(GL_TEXTURE_2D, font_data->tex_gl_ids[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, font_data->tex_sizes[i].x, font_data->tex_sizes[i].y, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, px_data);

            // The pixel data for this font texture is no longer needed in the arena, so rewind and allow it to be
            // overwritten.
            zfw_rewind_mem_arena(main_mem_arena);
        }
    }
    //

    return ZFW_TRUE;
}
