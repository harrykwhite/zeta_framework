#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <stb/stb_image.h>
#include <zfw_common.h>

#define ASSETS_FILE_NAME_MAX_LEN 64
#define ASSETS_FILE_REL_PATH_MAX_LEN 256

#define PACKING_INSTRS_FILE_NAME "zfw_asset_packing_instrs.json"
#define PACKING_INSTRS_TEXTURES_ARRAY_NAME "textures"
#define PACKING_INSTRS_SHADER_PROGS_ARRAY_NAME "shader_progs"
#define PACKING_INSTRS_VERT_SHADER_PROPERTY_NAME "vert_shader_rfp"
#define PACKING_INSTRS_FRAG_SHADER_PROPERTY_NAME "frag_shader_rfp"

static char *get_packing_instrs_file_chars()
{
	FILE *packing_instrs_file_fs = fopen(PACKING_INSTRS_FILE_NAME, "rb");

	if (!packing_instrs_file_fs)
	{
		zfw_log_error("Failed to open packing instructions JSON file \"%s\".", PACKING_INSTRS_FILE_NAME);
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

static zfw_bool_t pack_texture(const char *const tex_rfp, FILE *const assets_file_fs)
{
	int tex_width, tex_height;
	stbi_uc *const tex_px_data = stbi_load(tex_rfp, &tex_width, &tex_height, NULL, ZFW_TEX_CHANNEL_COUNT);

	if (!tex_px_data)
	{
		zfw_log_error("Failed to pack texture from file with relative path \"%s\".", tex_rfp);
		return ZFW_FALSE;
	}

	fwrite(&tex_width, sizeof(tex_width), 1, assets_file_fs);
	fwrite(&tex_height, sizeof(tex_height), 1, assets_file_fs);
	fwrite(tex_px_data, sizeof(*tex_px_data), tex_width * tex_height * ZFW_TEX_CHANNEL_COUNT, assets_file_fs);

	stbi_image_free(tex_px_data);

	zfw_log("Successfully packed texture from file with relative path \"%s\".", tex_rfp);

	return ZFW_TRUE;
}

static zfw_bool_t pack_shader(const char *const shader_rfp, FILE *const assets_file_fs)
{
	// TODO: Add error checking to determine whether shader source codes are valid.

	FILE *const shader_fs = fopen(shader_rfp, "rb");

	if (!shader_fs)
	{
		zfw_log_error("Failed to open shader file with relative path \"%s\".", shader_rfp);
		return ZFW_FALSE;
	}

	fseek(shader_fs, 0, SEEK_END);
	const int shader_src_size = ftell(shader_fs);

	if (shader_src_size > ZFW_SHADER_SRC_MAX_LEN)
	{
		zfw_log_error("The shader file with relative path \"%s\" exceeds the size limit of %d characters!", shader_rfp, ZFW_SHADER_SRC_MAX_LEN);
		fclose(shader_fs);
		return ZFW_FALSE;
	}

	char shader_src_buf[ZFW_SHADER_SRC_MAX_LEN] = "";

	fseek(shader_fs, 0, SEEK_SET);
	fread(shader_src_buf, sizeof(*shader_src_buf), shader_src_size, shader_fs);

	fwrite(shader_src_buf, sizeof(*shader_src_buf), sizeof(shader_src_buf) / sizeof(*shader_src_buf), assets_file_fs);

	fclose(shader_fs);

	zfw_log("Successfully packed shader from file with relative path \"%s\".", shader_rfp);

	return ZFW_TRUE;
}

static void tidy_up(const zfw_bool_t packing_successful, char *const packing_instrs_json_file_chars, FILE *const assets_file_fs, cJSON *const c_json, const char *const assets_file_rel_path)
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

	free(packing_instrs_json_file_chars);
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

	// Determine the asset file relative path.
	char assets_file_rel_path[ASSETS_FILE_REL_PATH_MAX_LEN] = { 0 };

	for (int i = 0; i < ASSETS_FILE_REL_PATH_MAX_LEN; i++)
	{
		if (assets_file_dir[i] != '\0')
		{
			assets_file_rel_path[i] = assets_file_dir[i];
		}
		else
		{
			assets_file_rel_path[i] = '/';
			strncpy(assets_file_rel_path + i + 1, ZFW_ASSETS_FILE_NAME, ASSETS_FILE_REL_PATH_MAX_LEN - i + 1);
			break;
		}
	}

	if (assets_file_rel_path[ASSETS_FILE_REL_PATH_MAX_LEN - 1] != '\0')
	{
		zfw_log_error("The provided output directory path of \"%s\" is too large!", assets_file_dir);
		return EXIT_FAILURE;
	}

	// Get the contents of the packing instructions JSON file.
	char *packing_instrs_file_chars = get_packing_instrs_file_chars();

	if (!packing_instrs_file_chars)
	{
		return EXIT_FAILURE;
	}

	// Create or open the assets file.
	FILE *const assets_file_fs = fopen(assets_file_rel_path, "wb");

	if (!assets_file_fs)
	{
		zfw_log_error("Failed to create or open assets file \"%s\".", assets_file_rel_path);
		tidy_up(ZFW_FALSE, packing_instrs_file_chars, NULL, NULL, assets_file_rel_path);
		return EXIT_FAILURE;
	}

	// Parse the packing instructions JSON file contents.
	cJSON *const c_json = cJSON_Parse(packing_instrs_file_chars);

	if (!c_json)
	{
		zfw_log_error("cJSON failed to parse packing instructions JSON file contents!");
		tidy_up(ZFW_FALSE, packing_instrs_file_chars, assets_file_fs, NULL, assets_file_rel_path);
		return EXIT_FAILURE;
	}

	// Pack assets based on information in the JSON file.
	for (int i = 0; i < 2; i++)
	{
		// Get the array for this asset type.
		const char *cj_array_name;

		switch (i)
		{
			case 0: cj_array_name = PACKING_INSTRS_TEXTURES_ARRAY_NAME; break;
			case 1: cj_array_name = PACKING_INSTRS_SHADER_PROGS_ARRAY_NAME; break;
		}

		cJSON *cj_assets = cJSON_GetObjectItemCaseSensitive(c_json, cj_array_name);

		const zfw_bool_t cj_array_found = cJSON_IsArray(cj_assets);

		// Get and write the asset count to the assets file.
		const int asset_count = cj_array_found ? cJSON_GetArraySize(cj_assets) : 0;
		fwrite(&asset_count, sizeof(asset_count), 1, assets_file_fs);
		//

		if (!cj_array_found)
		{
			zfw_log("Did not find array with name \"%s\" in \"%s\". Continuing.", cj_array_name, PACKING_INSTRS_FILE_NAME);

			// Get and write the asset count to the assets file.
			const int asset_count = 0;
			fwrite(&asset_count, sizeof(asset_count), 1, assets_file_fs);

			continue;
		}

		// Write the data of each asset of this asset type.
		cJSON *cj_asset = NULL;

		cJSON_ArrayForEach(cj_asset, cj_assets)
		{
			switch (i)
			{
				case 0:
					if (!cJSON_IsString(cj_asset))
					{
						zfw_log_error("In \"%s\", an element in the array with name \"%s\" is not a string!", PACKING_INSTRS_FILE_NAME, cj_array_name);
						tidy_up(ZFW_FALSE, packing_instrs_file_chars, assets_file_fs, c_json, assets_file_rel_path);
						return EXIT_FAILURE;
					}

					if (!pack_texture(cj_asset->valuestring, assets_file_fs))
					{
						tidy_up(ZFW_FALSE, packing_instrs_file_chars, assets_file_fs, c_json, assets_file_rel_path);
						return EXIT_FAILURE;
					}

					break;

				case 1:
					cJSON * cj_prog_vert_shader_rfp = cJSON_GetObjectItemCaseSensitive(cj_asset, PACKING_INSTRS_VERT_SHADER_PROPERTY_NAME);
					cJSON *cj_prog_frag_shader_rfp = cJSON_GetObjectItemCaseSensitive(cj_asset, PACKING_INSTRS_FRAG_SHADER_PROPERTY_NAME);

					if (!cJSON_IsString(cj_prog_vert_shader_rfp) || !cJSON_IsString(cj_prog_frag_shader_rfp))
					{
						zfw_log_error("Failed to find string properties \"%s\" and \"%s\" in an object in the \"%s\" array in \"%s\".", PACKING_INSTRS_VERT_SHADER_PROPERTY_NAME, PACKING_INSTRS_FRAG_SHADER_PROPERTY_NAME, cj_array_name, PACKING_INSTRS_FILE_NAME);
						tidy_up(ZFW_FALSE, packing_instrs_file_chars, assets_file_fs, c_json, assets_file_rel_path);
						return EXIT_FAILURE;
					}

					if (!pack_shader(cj_prog_vert_shader_rfp->valuestring, assets_file_fs) || pack_shader(cj_prog_frag_shader_rfp->valuestring, assets_file_fs))
					{
						tidy_up(ZFW_FALSE, packing_instrs_file_chars, assets_file_fs, c_json, assets_file_rel_path);
						return EXIT_FAILURE;
					}

					break;
			}
		}
	}

	tidy_up(ZFW_TRUE, packing_instrs_file_chars, assets_file_fs, c_json, assets_file_rel_path);

	return EXIT_SUCCESS;
}
