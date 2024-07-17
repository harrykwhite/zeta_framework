#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <external/stb_image.h>
#include <zfw_common.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define SRC_ASSET_FILE_PATH_BUF_SIZE 256
#define ASSETS_FILE_PATH_BUF_SIZE 256

#define PACKING_INSTRS_FILE_NAME "zfw_asset_packing_instrs.json"

#define FONT_PT_SIZE_MIN 11
#define FONT_PT_SIZE_MAX 144

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

static char *get_packing_instrs_file_chars(char src_asset_file_path_buf[SRC_ASSET_FILE_PATH_BUF_SIZE], const int src_asset_file_path_start_len)
{
    strncpy(src_asset_file_path_buf + src_asset_file_path_start_len, PACKING_INSTRS_FILE_NAME, SRC_ASSET_FILE_PATH_BUF_SIZE - src_asset_file_path_start_len);

    if (src_asset_file_path_buf[SRC_ASSET_FILE_PATH_BUF_SIZE - 1])
    {
        zfw_log_error("The packing instructions file name is too long!");
        return NULL;
    }

    FILE *const packing_instrs_file_fs = fopen(src_asset_file_path_buf, "rb");

    if (!packing_instrs_file_fs)
    {
        zfw_log_error("Failed to open packing instructions file \"%s\".", PACKING_INSTRS_FILE_NAME);
        return NULL;
    }

    fseek(packing_instrs_file_fs, 0, SEEK_END);
    const int packing_instrs_file_size = ftell(packing_instrs_file_fs);

    const int packing_instrs_file_chars_size = packing_instrs_file_size + 1; // (A null terminator must be added to the end.)
    char *packing_instrs_file_chars = malloc(packing_instrs_file_chars_size);

    if (!packing_instrs_file_chars)
    {
        zfw_log_error("Failed to allocate %d bytes to store the contents of \"%s\".", packing_instrs_file_chars_size, PACKING_INSTRS_FILE_NAME);
        fclose(packing_instrs_file_fs);
        return NULL;
    }

    fseek(packing_instrs_file_fs, 0, SEEK_SET);
    fread(packing_instrs_file_chars, 1, packing_instrs_file_size, packing_instrs_file_fs);

    fclose(packing_instrs_file_fs);

    packing_instrs_file_chars[packing_instrs_file_chars_size - 1] = '\0';

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

static zfw_bool_t pack_textures(cJSON *const c_json, char src_asset_file_path_buf[SRC_ASSET_FILE_PATH_BUF_SIZE], const int src_asset_file_path_start_len, FILE *const assets_file_fs)
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

        strncpy(src_asset_file_path_buf + src_asset_file_path_start_len, cj_tex->valuestring, SRC_ASSET_FILE_PATH_BUF_SIZE - src_asset_file_path_start_len);

        if (src_asset_file_path_buf[SRC_ASSET_FILE_PATH_BUF_SIZE - 1])
        {
            zfw_log_error("The texture relative file path of \"%s\" exceeds the size limit of %d characters!", cj_tex->valuestring, SRC_ASSET_FILE_PATH_BUF_SIZE - 1 - src_asset_file_path_start_len);
            return ZFW_FALSE;
        }

        zfw_vec_2d_i_t tex_size;
        stbi_uc *const tex_px_data = stbi_load(src_asset_file_path_buf, &tex_size.x, &tex_size.y, NULL, ZFW_TEX_CHANNEL_COUNT);

        if (!tex_px_data)
        {
            zfw_log_error("Failed to load pixel data for texture with relative file path \"%s\"!", cj_tex->valuestring);
            return ZFW_FALSE;
        }

        fwrite(&tex_size, sizeof(tex_size), 1, assets_file_fs);
        fwrite(tex_px_data, sizeof(*tex_px_data), tex_size.x * tex_size.y * ZFW_TEX_CHANNEL_COUNT, assets_file_fs);

        stbi_image_free(tex_px_data);
    }

    return ZFW_TRUE;
}

static zfw_bool_t pack_shader_progs(cJSON *const c_json, char src_asset_file_path_buf[SRC_ASSET_FILE_PATH_BUF_SIZE], const int src_asset_file_path_start_len, FILE *const assets_file_fs)
{
    const cJSON *const cj_progs = get_cj_assets_array_and_write_asset_count_to_assets_file(c_json, "shader_progs", assets_file_fs);

    if (!cj_progs)
    {
        return ZFW_TRUE;
    }

    const cJSON *cj_prog = NULL;

    cJSON_ArrayForEach(cj_prog, cj_progs)
    {
        const cJSON *const cj_vert_shader_rfp = cJSON_GetObjectItemCaseSensitive(cj_prog, "vert_shader_rfp");
        const cJSON *const cj_frag_shader_rfp = cJSON_GetObjectItemCaseSensitive(cj_prog, "frag_shader_rfp");

        if (!cJSON_IsString(cj_vert_shader_rfp) || !cJSON_IsString(cj_frag_shader_rfp))
        {
            return ZFW_FALSE;
        }

        for (int i = 0; i < 2; i++)
        {
            const char *const shader_file_rel_path = i == 0 ? cj_vert_shader_rfp->valuestring : cj_frag_shader_rfp->valuestring;

            strncpy(src_asset_file_path_buf + src_asset_file_path_start_len, shader_file_rel_path, SRC_ASSET_FILE_PATH_BUF_SIZE - src_asset_file_path_start_len);

            if (src_asset_file_path_buf[SRC_ASSET_FILE_PATH_BUF_SIZE - 1])
            {
                zfw_log_error("The shader file relative path of \"%s\" exceeds the size limit of %d characters!", shader_file_rel_path, SRC_ASSET_FILE_PATH_BUF_SIZE - 1 - src_asset_file_path_start_len);
                return ZFW_FALSE;
            }

            FILE *const shader_file_fs = fopen(src_asset_file_path_buf, "rb");

            if (!shader_file_fs)
            {
                zfw_log_error("Failed to open shader file \"%s\"!", src_asset_file_path_buf);
                return ZFW_FALSE;
            }

            fseek(shader_file_fs, 0, SEEK_END);
            const int shader_file_size = ftell(shader_file_fs);

            if (shader_file_size + 1 > ZFW_SHADER_SRC_BUF_SIZE)
            {
                zfw_log_error("The size of shader file \"%s\" exceeds the limit of %d bytes!", src_asset_file_path_buf, ZFW_SHADER_SRC_BUF_SIZE);
                fclose(shader_file_fs);
                return ZFW_FALSE;
            }

            char shader_src_buf[ZFW_SHADER_SRC_BUF_SIZE] = {0};

            fseek(shader_file_fs, 0, SEEK_SET);
            fread(shader_src_buf, 1, shader_file_size, shader_file_fs);

            fwrite(shader_src_buf, 1, sizeof(shader_src_buf), assets_file_fs);

            fclose(shader_file_fs);
        }
    }

    return ZFW_TRUE;
}

static zfw_bool_t pack_fonts(cJSON *const c_json, char src_asset_file_path_buf[SRC_ASSET_FILE_PATH_BUF_SIZE], const int src_asset_file_path_start_len, FILE *const assets_file_fs)
{
    const cJSON *const cj_fonts = get_cj_assets_array_and_write_asset_count_to_assets_file(c_json, "fonts", assets_file_fs);

    if (!cj_fonts)
    {
        return ZFW_TRUE;
    }

    const int cj_fonts_len = cJSON_GetArraySize(cj_fonts);

    // Set up buffers for font data.
    zfw_mem_arena_t mem_arena;
    zfw_init_mem_arena(&mem_arena, (1 << 20) * 10);

    int *line_heights = zfw_mem_arena_alloc(&mem_arena, sizeof(*line_heights) * cj_fonts_len);

    if (!line_heights)
    {
        zfw_clean_mem_arena(&mem_arena);
        return ZFW_FALSE;
    }

    font_char_hor_offs_t *char_hor_offsets = zfw_mem_arena_alloc(&mem_arena, sizeof(*char_hor_offsets) * ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len);

    if (!char_hor_offsets)
    {
        zfw_clean_mem_arena(&mem_arena);
        return ZFW_FALSE;
    }

    font_char_vert_offs_t *char_vert_offsets = zfw_mem_arena_alloc(&mem_arena, sizeof(*char_vert_offsets) * ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len);

    if (!char_vert_offsets)
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

    font_char_src_rect_t *char_src_rects = zfw_mem_arena_alloc(&mem_arena, sizeof(*char_src_rects) * ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len);

    if (!char_src_rects)
    {
        zfw_clean_mem_arena(&mem_arena);
        return ZFW_FALSE;
    }

    font_char_kerning_t *char_kernings = zfw_mem_arena_alloc(&mem_arena, sizeof(*char_kernings) * ZFW_FONT_CHAR_RANGE_SIZE * ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len);

    if (!char_kernings)
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
    const cJSON *cj_font = NULL;

    int i = 0;

    cJSON_ArrayForEach(cj_font, cj_fonts)
    {
        const cJSON *const cj_rfp = cJSON_GetObjectItemCaseSensitive(cj_font, "rfp");
        const cJSON *const cj_pt_size = cJSON_GetObjectItemCaseSensitive(cj_font, "pt_size");

        if (!cJSON_IsString(cj_rfp) || !cJSON_IsNumber(cj_pt_size))
        {
            zfw_clean_mem_arena(&mem_arena);
            return ZFW_FALSE;
        }

        strncpy(src_asset_file_path_buf + src_asset_file_path_start_len, cj_rfp->valuestring, SRC_ASSET_FILE_PATH_BUF_SIZE - src_asset_file_path_start_len);

        if (src_asset_file_path_buf[SRC_ASSET_FILE_PATH_BUF_SIZE - 1])
        {
            zfw_log_error("The font relative file path of \"%s\" exceeds the size limit of %d characters!", cj_rfp->valuestring, SRC_ASSET_FILE_PATH_BUF_SIZE - 1 - src_asset_file_path_start_len);
            zfw_clean_mem_arena(&mem_arena);
            return ZFW_FALSE;
        }

        if (cj_pt_size->valueint < FONT_PT_SIZE_MIN || cj_pt_size->valueint > FONT_PT_SIZE_MAX)
        {
            zfw_log_error("Font point sizes must be between %d and %d inclusive!", FONT_PT_SIZE_MIN, FONT_PT_SIZE_MAX);

            FT_Done_FreeType(ft_lib);
            zfw_clean_mem_arena(&mem_arena);

            return ZFW_FALSE;
        }

        // Set up the font face.
        FT_Face ft_face;

        if (FT_New_Face(ft_lib, src_asset_file_path_buf, 0, &ft_face))
        {
            zfw_log_error("Failed to set up the FreeType face object for font with relative path \"%s\".", cj_rfp->valuestring);

            FT_Done_FreeType(ft_lib);
            zfw_clean_mem_arena(&mem_arena);

            return ZFW_FALSE;
        }

        FT_Set_Char_Size(ft_face, cj_pt_size->valueint << 6, 0, 96, 0);
        //

        line_heights[i] = ft_face->size->metrics.height >> 6;

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

        tex_sizes[i].x = ZFW_MIN(ideal_tex_width, max_tex_width);
        tex_sizes[i].y = line_heights[i] * ((ideal_tex_width / max_tex_width) + 1);

        // Initialize the pixel data of the font texture by setting all the pixels to be transparent white.
        const int tex_px_data_size = tex_sizes[i].x * tex_sizes[i].y * ZFW_FONT_TEX_CHANNEL_COUNT;
        tex_px_datas[i] = zfw_mem_arena_alloc(&mem_arena, tex_px_data_size);

        if (!tex_px_datas[i])
        {
            FT_Done_Face(ft_face);
            FT_Done_FreeType(ft_lib);

            return ZFW_FALSE;
        }

        for (int j = (tex_sizes[i].x * tex_sizes[i].y) - 1; j >= 0; j--)
        {
            const int px_data_index = j * 4;

            tex_px_datas[i][px_data_index + 0] = 255;
            tex_px_datas[i][px_data_index + 1] = 255;
            tex_px_datas[i][px_data_index + 2] = 255;
            tex_px_datas[i][px_data_index + 3] = 0;
        }

        // Get and store information for all font characters.
        int char_draw_x = 0;
        int char_draw_y = 0;

        for (int j = 0; j < ZFW_FONT_CHAR_RANGE_SIZE; j++)
        {
            const FT_UInt ft_char_index = FT_Get_Char_Index(ft_face, ZFW_FONT_CHAR_RANGE_BEGIN + j);

            FT_Load_Glyph(ft_face, ft_char_index, FT_LOAD_DEFAULT);
            FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL);

            if (char_draw_x + ft_face->glyph->bitmap.width > max_tex_width)
            {
                char_draw_x = 0;
                char_draw_y += line_heights[i];
            }

            const int font_char_index = (i * ZFW_FONT_CHAR_RANGE_SIZE) + j;

            char_hor_offsets[font_char_index] = ft_face->glyph->metrics.horiBearingX >> 6;
            char_vert_offsets[font_char_index] = (ft_face->size->metrics.ascender - ft_face->glyph->metrics.horiBearingY) >> 6;

            chars_hor_advances[font_char_index] = ft_face->glyph->metrics.horiAdvance >> 6;

            char_src_rects[font_char_index].x = char_draw_x;
            char_src_rects[font_char_index].y = char_draw_y;
            char_src_rects[font_char_index].width = ft_face->glyph->bitmap.width;
            char_src_rects[font_char_index].height = ft_face->glyph->bitmap.rows;

            for (int k = 0; k < ZFW_FONT_CHAR_RANGE_SIZE; k++)
            {
                FT_Vector ft_kerning;
                FT_Get_Kerning(ft_face, FT_Get_Char_Index(ft_face, ZFW_FONT_CHAR_RANGE_BEGIN + k), ft_char_index, FT_KERNING_DEFAULT, &ft_kerning);

                char_kernings[(ZFW_FONT_CHAR_RANGE_SIZE * font_char_index) + k] = ft_kerning.x >> 6;
            }

            // Update the font texture's pixel data with the character.
            for (int y = 0; y < char_src_rects[font_char_index].height; y++)
            {
                for (int x = 0; x < char_src_rects[font_char_index].width; x++)
                {
                    const unsigned char px_alpha = ft_face->glyph->bitmap.buffer[(y * ft_face->glyph->bitmap.width) + x];

                    if (px_alpha > 0)
                    {
                        const int px_data_index = ((char_src_rects[font_char_index].y + y) * tex_sizes[i].x * ZFW_FONT_TEX_CHANNEL_COUNT) + ((char_src_rects[font_char_index].x + x) * ZFW_FONT_TEX_CHANNEL_COUNT);
                        tex_px_datas[i][px_data_index + 3] = px_alpha;
                    }
                }
            }

            char_draw_x += char_src_rects[font_char_index].width;
        }
        //

        FT_Done_Face(ft_face);

        i++;
    }
    //

    FT_Done_FreeType(ft_lib);

    // Write the font data to the file.
    fwrite(line_heights, sizeof(*line_heights), cj_fonts_len, assets_file_fs);

    fwrite(char_hor_offsets, sizeof(*char_hor_offsets), ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len, assets_file_fs);
    fwrite(char_vert_offsets, sizeof(*char_vert_offsets), ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len, assets_file_fs);

    fwrite(chars_hor_advances, sizeof(*chars_hor_advances), ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len, assets_file_fs);

    fwrite(char_src_rects, sizeof(*char_src_rects), ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len, assets_file_fs);

    fwrite(char_kernings, sizeof(*char_kernings), ZFW_FONT_CHAR_RANGE_SIZE * ZFW_FONT_CHAR_RANGE_SIZE * cj_fonts_len, assets_file_fs);

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

    // Get the source directory and the assets file directory if provided.
    if (argc != 3)
    {
        zfw_log_error("Invalid number of command-line arguments! Expected a source directory and an assets file directory to be provided.");
        return EXIT_FAILURE;
    }

    const char *const src_dir = argv[1];
    const char *const assets_file_dir = argv[2];

    // Determine the assets file path using the directory.
    char assets_file_path[ASSETS_FILE_PATH_BUF_SIZE];
    const int assets_file_path_len = snprintf(assets_file_path, ASSETS_FILE_PATH_BUF_SIZE, "%s/%s", assets_file_dir, ZFW_ASSETS_FILE_NAME);

    if (assets_file_path_len >= ASSETS_FILE_PATH_BUF_SIZE)
    {
        zfw_log_error("The provided assets file directory of \"%s\" is too long!", assets_file_dir);
        return EXIT_FAILURE;
    }

    // Initialize the source asset file path buffer with the source directory.
    char src_asset_file_path_buf[SRC_ASSET_FILE_PATH_BUF_SIZE] = {0};
    const int src_asset_file_path_start_len = snprintf(src_asset_file_path_buf, SRC_ASSET_FILE_PATH_BUF_SIZE, "%s/", src_dir);

    if (src_asset_file_path_start_len >= SRC_ASSET_FILE_PATH_BUF_SIZE)
    {
        zfw_log_error("The provided source directory of \"%s\" is too long!", src_dir);
        return EXIT_FAILURE;
    }

    // Get the contents of the packing instructions JSON file.
    char *const packing_instrs_file_chars = get_packing_instrs_file_chars(src_asset_file_path_buf, src_asset_file_path_start_len);

    if (!packing_instrs_file_chars)
    {
        return EXIT_FAILURE;
    }

    // Create or open the assets file.
    FILE *const assets_file_fs = fopen(assets_file_path, "wb");

    if (!assets_file_fs)
    {
        zfw_log_error("Failed to create or open assets file \"%s\".", assets_file_path);
        clean_up(ZFW_FALSE, packing_instrs_file_chars, NULL, NULL, assets_file_path);
        return EXIT_FAILURE;
    }

    // Parse the packing instructions file contents.
    cJSON *const c_json = cJSON_Parse(packing_instrs_file_chars);

    if (!c_json)
    {
        zfw_log_error("cJSON failed to parse packing instructions file contents!");
        clean_up(ZFW_FALSE, packing_instrs_file_chars, assets_file_fs, NULL, assets_file_path);
        return EXIT_FAILURE;
    }

    // Pack assets using the packing instructions file.
    if (!pack_textures(c_json, src_asset_file_path_buf, src_asset_file_path_start_len, assets_file_fs))
    {
        clean_up(ZFW_FALSE, packing_instrs_file_chars, assets_file_fs, c_json, assets_file_path);
        return EXIT_FAILURE;
    }

    if (!pack_shader_progs(c_json, src_asset_file_path_buf, src_asset_file_path_start_len, assets_file_fs))
    {
        clean_up(ZFW_FALSE, packing_instrs_file_chars, assets_file_fs, c_json, assets_file_path);
        return EXIT_FAILURE;
    }

    if (!pack_fonts(c_json, src_asset_file_path_buf, src_asset_file_path_start_len, assets_file_fs))
    {
        clean_up(ZFW_FALSE, packing_instrs_file_chars, assets_file_fs, c_json, assets_file_path);
        return EXIT_FAILURE;
    }
    //

    clean_up(ZFW_TRUE, packing_instrs_file_chars, assets_file_fs, c_json, assets_file_path);

    return EXIT_SUCCESS;
}
