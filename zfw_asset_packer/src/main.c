#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <zfw_common.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define ASSETS_FILE_NAME_BUF_SIZE 64
#define ASSETS_FILE_REL_PATH_BUF_SIZE 256

#define PACKING_INSTRS_FILE_NAME "zfw_asset_packing_instrs.json"

static void clean_up(const zfw_bool_t packing_successful, char *const packing_instrs_file_chars, FILE *const assets_file_fs, cJSON *const c_json, const char *const assets_file_rel_path)
{
    cJSON_Delete(c_json);

    if (assets_file_fs)
    {
        fclose(assets_file_fs);

        // Try to delete the assets file if packing failed.
        if (!packing_successful)
        {
            remove(assets_file_rel_path);
        }
    }

    free(packing_instrs_file_chars);
}

static char *get_packing_instrs_file_chars()
{
    FILE *packing_instrs_file_fs = fopen(PACKING_INSTRS_FILE_NAME, "rb");

    if (!packing_instrs_file_fs)
    {
        zfw_log_error("Failed to open packing instructions file \"%s\".", PACKING_INSTRS_FILE_NAME);
        return NULL;
    }

    fseek(packing_instrs_file_fs, 0, SEEK_END);
    const int packing_instrs_file_len = ftell(packing_instrs_file_fs);

    char *packing_instrs_file_chars = malloc(packing_instrs_file_len);

    if (!packing_instrs_file_chars)
    {
        zfw_log_error("Failed to allocate %d bytes to store the contents of \"%s\".", packing_instrs_file_len, PACKING_INSTRS_FILE_NAME);
        fclose(packing_instrs_file_fs);
        return NULL;
    }

    fseek(packing_instrs_file_fs, 0, SEEK_SET);
    fread(packing_instrs_file_chars, sizeof(*packing_instrs_file_chars), packing_instrs_file_len, packing_instrs_file_fs);

    fclose(packing_instrs_file_fs);

    return packing_instrs_file_chars;
}

static cJSON *get_cj_assets_array_and_write_asset_count_to_assets_file(cJSON *const c_json, const char *const packing_instrs_array_name, FILE *const assets_file_fs)
{
    cJSON *const cj_assets = cJSON_GetObjectItemCaseSensitive(c_json, packing_instrs_array_name);

    const zfw_bool_t cj_array_found = cJSON_IsArray(cj_assets);

    const int asset_count = cj_array_found ? cJSON_GetArraySize(cj_assets) : 0;
    fwrite(&asset_count, sizeof(asset_count), 1, assets_file_fs);

    if (!cj_array_found)
    {
        zfw_log_warning("Did not find array with name \"%s\" in \"%s\".", packing_instrs_array_name, PACKING_INSTRS_FILE_NAME);
        return NULL;
    }

    return cj_assets;
}

static zfw_bool_t pack_textures(cJSON *const c_json, FILE *const assets_file_fs)
{
    const cJSON *const cj_textures = get_cj_assets_array_and_write_asset_count_to_assets_file(c_json, "textures", assets_file_fs);

    if (!cj_textures)
    {
        return ZFW_TRUE;
    }

    const cJSON *cj_tex = NULL;

    cJSON_ArrayForEach(cj_tex, cj_textures)
    {
        if (!cJSON_IsString(cj_tex))
        {
            return ZFW_FALSE;
        }

        zfw_vec_2d_i_t tex_size;
        stbi_uc *const tex_px_data = stbi_load(cj_tex->valuestring, &tex_size.x, &tex_size.y, NULL, ZFW_TEX_CHANNEL_COUNT);

        if (!tex_px_data)
        {
            return ZFW_FALSE;
        }

        fwrite(&tex_size, sizeof(tex_size), 1, assets_file_fs);
        fwrite(tex_px_data, sizeof(*tex_px_data), tex_size.x * tex_size.y * ZFW_TEX_CHANNEL_COUNT, assets_file_fs);

        stbi_image_free(tex_px_data);
    }

    return ZFW_TRUE;
}

static zfw_bool_t pack_shader_progs(cJSON *const c_json, FILE *const assets_file_fs)
{
    const cJSON *const cj_progs = get_cj_assets_array_and_write_asset_count_to_assets_file(c_json, "shader_progs", assets_file_fs);

    if (!cj_progs)
    {
        return ZFW_TRUE;
    }

    const cJSON *cj_prog = NULL;

    cJSON_ArrayForEach(cj_prog, cj_progs)
    {
        cJSON *cj_vert_shader_rfp = cJSON_GetObjectItemCaseSensitive(cj_prog, "vert_shader_rfp");
        cJSON *cj_frag_shader_rfp = cJSON_GetObjectItemCaseSensitive(cj_prog, "frag_shader_rfp");

        if (!cJSON_IsString(cj_vert_shader_rfp) || !cJSON_IsString(cj_frag_shader_rfp))
        {
            return ZFW_FALSE;
        }

        for (int i = 0; i < 2; i++)
        {
            FILE *const shader_file_fs = fopen(i == 0 ? cj_vert_shader_rfp->valuestring : cj_frag_shader_rfp->valuestring, "rb");

            if (!shader_file_fs)
            {
                return ZFW_FALSE;
            }

            fseek(shader_file_fs, 0, SEEK_END);
            const int shader_src_size = ftell(shader_file_fs);

            if (shader_src_size > ZFW_SHADER_SRC_BUF_SIZE)
            {
                fclose(shader_file_fs);
                return ZFW_FALSE;
            }

            char shader_src_buf[ZFW_SHADER_SRC_BUF_SIZE] = { 0 };

            fseek(shader_file_fs, 0, SEEK_SET);
            fread(shader_src_buf, sizeof(*shader_src_buf), shader_src_size, shader_file_fs);

            fwrite(shader_src_buf, sizeof(*shader_src_buf), sizeof(shader_src_buf) / sizeof(*shader_src_buf), assets_file_fs);

            fclose(shader_file_fs);
        }
    }

    return ZFW_TRUE;
}

static zfw_bool_t pack_fonts(cJSON *const c_json, FILE *const assets_file_fs)
{
    const cJSON *const cj_fonts = get_cj_assets_array_and_write_asset_count_to_assets_file(c_json, "fonts", assets_file_fs);

    if (!cj_fonts)
    {
        return ZFW_TRUE;
    }

    const int cj_fonts_len = cJSON_GetArraySize(cj_fonts);

    // Set up buffers for font data.
    zfw_mem_arena_t mem_arena;
    zfw_init_mem_arena(&mem_arena, 1 << 20);

    int *line_heights = zfw_mem_arena_alloc(&mem_arena, sizeof(*line_heights) * cj_fonts_len);

    if (!line_heights)
    {
        zfw_clean_mem_arena(&mem_arena);
        return ZFW_FALSE;
    }

    font_char_hor_offs_t *chars_hor_offsets = zfw_mem_arena_alloc(&mem_arena, sizeof(*chars_hor_offsets) * ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len);

    if (!chars_hor_offsets)
    {
        zfw_clean_mem_arena(&mem_arena);
        return ZFW_FALSE;
    }

    font_char_vert_offs_t *chars_vert_offsets = zfw_mem_arena_alloc(&mem_arena, sizeof(*chars_vert_offsets) * ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len);

    if (!chars_vert_offsets)
    {
        zfw_clean_mem_arena(&mem_arena);
        return ZFW_FALSE;
    }

    font_char_hor_advance_t *chars_hor_advances = zfw_mem_arena_alloc(&mem_arena, sizeof(*chars_hor_advances) * ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len);

    if (!chars_hor_advances)
    {
        zfw_clean_mem_arena(&mem_arena);
        return ZFW_FALSE;
    }

    font_char_src_rect_t *chars_src_rects = zfw_mem_arena_alloc(&mem_arena, sizeof(*chars_src_rects) * ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len);

    if (!chars_src_rects)
    {
        zfw_clean_mem_arena(&mem_arena);
        return ZFW_FALSE;
    }

    font_char_kerning_t *chars_kernings = zfw_mem_arena_alloc(&mem_arena, sizeof(*chars_kernings) * ZFW_FONT_CHAR_RANGE_SIZE * ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len);

    if (!chars_kernings)
    {
        zfw_clean_mem_arena(&mem_arena);
        return ZFW_FALSE;
    }

    zfw_vec_2d_i_t *tex_sizes = zfw_mem_arena_alloc(&mem_arena, sizeof(*tex_sizes) * cj_fonts_len);

    if (!tex_sizes)
    {
        zfw_clean_mem_arena(&mem_arena);
        return ZFW_FALSE;
    }

    unsigned char **tex_px_datas = zfw_mem_arena_alloc(&mem_arena, sizeof(*tex_px_datas) * cj_fonts_len);

    if (!tex_px_datas)
    {
        zfw_clean_mem_arena(&mem_arena);
        return ZFW_FALSE;
    }

    // Initialize FreeType.
    FT_Library ft_lib;

    if (FT_Init_FreeType(&ft_lib))
    {
        zfw_log_error("Failed to initialize FreeType!");
        return ZFW_FALSE;
    }

    // Iterate through each font array element.
    int font_index = 0;

    const cJSON *cj_font = NULL;

    cJSON_ArrayForEach(cj_font, cj_fonts)
    {
        cJSON *cj_rfp = cJSON_GetObjectItemCaseSensitive(cj_font, "rfp");
        cJSON *cj_pt_size = cJSON_GetObjectItemCaseSensitive(cj_font, "pt_size");

        if (!cJSON_IsString(cj_rfp) || !cJSON_IsNumber(cj_pt_size))
        {
            zfw_clean_mem_arena(&mem_arena);
            return ZFW_FALSE;
        }

        // TODO: Check that the point size is valid.

        // Set up the font face.
        FT_Face ft_face;

        if (FT_New_Face(ft_lib, cj_rfp->valuestring, 0, &ft_face))
        {
            FT_Done_FreeType(ft_lib);
            return ZFW_FALSE;
        }

        FT_Set_Char_Size(ft_face, cj_pt_size->valueint << 6, 0, 96, 0);
        //

        line_heights[font_index] = ft_face->size->metrics.height >> 6;

        // Get the largest bitmap width of a glyph.
        int largest_glyph_bitmap_width = 0;

        for (int j = 0; j < ZFW_FONT_CHAR_RANGE_SIZE; j++)
        {
            FT_Load_Glyph(ft_face, FT_Get_Char_Index(ft_face, ZFW_FONT_CHAR_RANGE_BEGIN + j), FT_LOAD_DEFAULT);
            FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL);

            largest_glyph_bitmap_width = ZFW_MAX(ft_face->glyph->bitmap.width, largest_glyph_bitmap_width);
        }

        // Set the ideal width of the font texture based on the largest glyph bitmap width.
        const int ideal_tex_width = largest_glyph_bitmap_width * ZFW_FONT_CHAR_RANGE_SIZE;

        // Determine the actual font texture size based on the ideal width and a maximum width.
        const int max_tex_width = 1024;

        tex_sizes[font_index].x = ZFW_MIN(ideal_tex_width, max_tex_width);
        tex_sizes[font_index].y = line_heights[font_index] * ((ideal_tex_width / max_tex_width) + 1);

        // Initialize the pixel data of the font texture by setting all the pixels to be transparent white.
        const int tex_px_data_size = tex_sizes[font_index].x * tex_sizes[font_index].y * ZFW_FONT_TEX_CHANNEL_COUNT;
        tex_px_datas[font_index] = (unsigned char *)malloc(tex_px_data_size);

        if (!tex_px_datas[font_index])
        {
            FT_Done_Face(ft_face);
            FT_Done_FreeType(ft_lib);

            return ZFW_FALSE;
        }

        for (int i = (tex_sizes[font_index].x * tex_sizes[font_index].y) - 1; i >= 0; i--)
        {
            const int px_data_index = i * 4;

            tex_px_datas[font_index][px_data_index + 0] = 255;
            tex_px_datas[font_index][px_data_index + 1] = 255;
            tex_px_datas[font_index][px_data_index + 2] = 255;
            tex_px_datas[font_index][px_data_index + 3] = 0;
        }

        // Get and store information for all font characters.
        int char_draw_x = 0;
        int char_draw_y = 0;

        for (int i = 0; i < ZFW_FONT_CHAR_RANGE_SIZE; i++)
        {
            const FT_UInt ft_char_index = FT_Get_Char_Index(ft_face, ZFW_FONT_CHAR_RANGE_BEGIN + i);

            FT_Load_Glyph(ft_face, ft_char_index, FT_LOAD_DEFAULT);
            FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL);

            if (char_draw_x + ft_face->glyph->bitmap.width > max_tex_width)
            {
                char_draw_x = 0;
                char_draw_y += line_heights[font_index];
            }

            const int char_index = (font_index * ZFW_FONT_CHAR_RANGE_SIZE) + i;

            chars_hor_offsets[char_index] = ft_face->glyph->metrics.horiBearingX >> 6;
            chars_vert_offsets[char_index] = (ft_face->size->metrics.ascender - ft_face->glyph->metrics.horiBearingY) >> 6;

            chars_hor_advances[char_index] = ft_face->glyph->metrics.horiAdvance >> 6;

            chars_src_rects[char_index].x = char_draw_x;
            chars_src_rects[char_index].y = char_draw_y;
            chars_src_rects[char_index].width = ft_face->glyph->bitmap.width;
            chars_src_rects[char_index].height = ft_face->glyph->bitmap.rows;

            for (int j = 0; j < ZFW_FONT_CHAR_RANGE_SIZE; j++)
            {
                FT_Vector ft_kerning;
                FT_Get_Kerning(ft_face, FT_Get_Char_Index(ft_face, ZFW_FONT_CHAR_RANGE_BEGIN + j), ft_char_index, FT_KERNING_DEFAULT, &ft_kerning);

                chars_kernings[(font_index * ZFW_FONT_CHAR_RANGE_SIZE * ZFW_FONT_CHAR_RANGE_SIZE) + (i * ZFW_FONT_CHAR_RANGE_SIZE) + j] = ft_kerning.x >> 6;
            }

            // Update the font texture's pixel data with the character.
            for (int y = 0; y < chars_src_rects[char_index].height; y++)
            {
                for (int x = 0; x < chars_src_rects[char_index].width; x++)
                {
                    const unsigned char px_alpha = ft_face->glyph->bitmap.buffer[(y * ft_face->glyph->bitmap.width) + x];

                    if (px_alpha > 0)
                    {
                        const int px_data_index = ((chars_src_rects[char_index].y + y) * tex_sizes[font_index].x * ZFW_FONT_TEX_CHANNEL_COUNT) + ((chars_src_rects[char_index].x + x) * ZFW_FONT_TEX_CHANNEL_COUNT);
                        tex_px_datas[font_index][px_data_index + 3] = px_alpha;
                    }
                }
            }

            char_draw_x += chars_src_rects[char_index].width;
        }
        //

        FT_Done_Face(ft_face);

        font_index++;
    }
    //

    FT_Done_FreeType(ft_lib);

    // Write the font data to the file.
    fwrite(line_heights, sizeof(*line_heights), cj_fonts_len, assets_file_fs);

    fwrite(chars_hor_offsets, sizeof(*chars_hor_offsets), ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len, assets_file_fs);
    fwrite(chars_vert_offsets, sizeof(*chars_vert_offsets), ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len, assets_file_fs);

    fwrite(chars_hor_advances, sizeof(*chars_hor_advances), ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len, assets_file_fs);

    fwrite(chars_src_rects, sizeof(*chars_src_rects), ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len, assets_file_fs);

    fwrite(chars_kernings, sizeof(*chars_kernings), ZFW_FONT_CHAR_RANGE_SIZE * ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len, assets_file_fs);

    fwrite(tex_sizes, sizeof(*tex_sizes), cj_fonts_len, assets_file_fs);

    for (int i = 0; i < cj_fonts_len; i++)
    {
        fwrite(tex_px_datas[i], sizeof(*tex_px_datas[i]), tex_sizes[i].x * tex_sizes[i].y * ZFW_FONT_TEX_CHANNEL_COUNT, assets_file_fs);
    }
    //

    zfw_clean_mem_arena(&mem_arena);

    return ZFW_TRUE;
}

int main(int argc, char *argv[])
{
    // Ensure data type sizes meet requirements before proceeding.
    if (!zfw_check_data_type_sizes())
    {
        return EXIT_FAILURE;
    }

    // Set the assets file directory, or get it if it was passed as a command-line argument.
    const char *assets_file_dir = "";

    if (argc == 2)
    {
        assets_file_dir = argv[1];
    }

    // Determine the assets file relative path.
    char assets_file_rel_path[ASSETS_FILE_REL_PATH_BUF_SIZE] = { 0 };

    for (int i = 0; i < ASSETS_FILE_REL_PATH_BUF_SIZE; i++)
    {
        if (assets_file_dir[i] != '\0')
        {
            assets_file_rel_path[i] = assets_file_dir[i];
        }
        else
        {
            assets_file_rel_path[i] = '/';
            strncpy(assets_file_rel_path + i + 1, ZFW_ASSETS_FILE_NAME, ASSETS_FILE_REL_PATH_BUF_SIZE - i + 1);
            break;
        }
    }

    if (assets_file_rel_path[ASSETS_FILE_REL_PATH_BUF_SIZE - 1] != '\0')
    {
        zfw_log_error("The provided assets file directory path of \"%s\" is too large!", assets_file_dir);
        return EXIT_FAILURE;
    }

    // Get the contents of the packing instructions JSON file.
    char *const packing_instrs_file_chars = get_packing_instrs_file_chars();

    if (!packing_instrs_file_chars)
    {
        return EXIT_FAILURE;
    }

    // Create or open the assets file.
    FILE *const assets_file_fs = fopen(assets_file_rel_path, "wb");

    if (!assets_file_fs)
    {
        zfw_log_error("Failed to create or open assets file \"%s\".", assets_file_rel_path);
        clean_up(ZFW_FALSE, packing_instrs_file_chars, NULL, NULL, assets_file_rel_path);
        return EXIT_FAILURE;
    }

    // Parse the packing instructions file contents.
    cJSON *const c_json = cJSON_Parse(packing_instrs_file_chars);

    if (!c_json)
    {
        zfw_log_error("cJSON failed to parse packing instructions file contents!");
        clean_up(ZFW_FALSE, packing_instrs_file_chars, assets_file_fs, NULL, assets_file_rel_path);
        return EXIT_FAILURE;
    }

    // Pack assets using the packing instructions file.
    if (!pack_textures(c_json, assets_file_fs))
    {
        clean_up(ZFW_FALSE, packing_instrs_file_chars, assets_file_fs, c_json, assets_file_rel_path);
        return EXIT_FAILURE;
    }

    if (!pack_shader_progs(c_json, assets_file_fs))
    {
        clean_up(ZFW_FALSE, packing_instrs_file_chars, assets_file_fs, c_json, assets_file_rel_path);
        return EXIT_FAILURE;
    }

    if (!pack_fonts(c_json, assets_file_fs))
    {
        clean_up(ZFW_FALSE, packing_instrs_file_chars, assets_file_fs, c_json, assets_file_rel_path);
        return EXIT_FAILURE;
    }
    //

    clean_up(ZFW_TRUE, packing_instrs_file_chars, assets_file_fs, c_json, assets_file_rel_path);

    return EXIT_SUCCESS;
}
