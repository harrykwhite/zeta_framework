#include <zfw.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GLFW/glfw3.h>

////// Globals //////
zfw_mem_arena_t zfw_g_main_mem_arena;
zfw_mem_arena_t zfw_g_tick_mem_arena;

////// Internal Structs //////

// Pointers to data to be cleaned up on game end. If a pointer is null, it should be because the data hasn't been set up yet and thus does not need to be cleaned.
typedef struct
{
	GLFWwindow *glfw_window;

	zfw_user_asset_data_t *user_asset_data;
	zfw_builtin_shader_prog_data_t *builtin_shader_prog_data;

	zfw_sprite_batch_data_t *sprite_batch_datas;
	int sprite_batch_datas_cleanup_count;
} game_cleanup_data_t;

typedef struct
{
	zfw_window_state_t *const window_state;
	zfw_input_state_t *const input_state;
} glfw_window_callback_data_t;

////// Static GLFW Callback Functions //////
static void glfw_window_size_callback(GLFWwindow *glfw_window, int width, int height)
{
	glfw_window_callback_data_t *const callback_data = glfwGetWindowUserPointer(glfw_window);
	callback_data->window_state->size = zfw_create_vec_2d_i(width, height);

	glViewport(0, 0, width, height);
}

static void glfw_window_pos_callback(GLFWwindow *glfw_window, int x, int y)
{
	glfw_window_callback_data_t *const callback_data = glfwGetWindowUserPointer(glfw_window);
	callback_data->window_state->pos = zfw_create_vec_2d_i(x, y);
}

static void glfw_key_callback(GLFWwindow *glfw_window, int glfw_key_index, int glfw_scancode, int glfw_action, int glfw_mods)
{
	if (glfw_action == GLFW_REPEAT)
	{
		return;
	}

	int key_code = -1;

	switch (glfw_key_index)
	{
		case GLFW_KEY_SPACE: key_code = ZFW_KEY_CODE__SPACE; break;

		case GLFW_KEY_0: key_code = ZFW_KEY_CODE__0; break;
		case GLFW_KEY_1: key_code = ZFW_KEY_CODE__1; break;
		case GLFW_KEY_2: key_code = ZFW_KEY_CODE__2; break;
		case GLFW_KEY_3: key_code = ZFW_KEY_CODE__3; break;
		case GLFW_KEY_4: key_code = ZFW_KEY_CODE__4; break;
		case GLFW_KEY_5: key_code = ZFW_KEY_CODE__5; break;
		case GLFW_KEY_6: key_code = ZFW_KEY_CODE__6; break;
		case GLFW_KEY_7: key_code = ZFW_KEY_CODE__7; break;
		case GLFW_KEY_8: key_code = ZFW_KEY_CODE__8; break;
		case GLFW_KEY_9: key_code = ZFW_KEY_CODE__9; break;

		case GLFW_KEY_A: key_code = ZFW_KEY_CODE__A; break;
		case GLFW_KEY_B: key_code = ZFW_KEY_CODE__B; break;
		case GLFW_KEY_C: key_code = ZFW_KEY_CODE__C; break;
		case GLFW_KEY_D: key_code = ZFW_KEY_CODE__D; break;
		case GLFW_KEY_E: key_code = ZFW_KEY_CODE__E; break;
		case GLFW_KEY_F: key_code = ZFW_KEY_CODE__F; break;
		case GLFW_KEY_G: key_code = ZFW_KEY_CODE__G; break;
		case GLFW_KEY_H: key_code = ZFW_KEY_CODE__H; break;
		case GLFW_KEY_I: key_code = ZFW_KEY_CODE__I; break;
		case GLFW_KEY_J: key_code = ZFW_KEY_CODE__J; break;
		case GLFW_KEY_K: key_code = ZFW_KEY_CODE__K; break;
		case GLFW_KEY_L: key_code = ZFW_KEY_CODE__L; break;
		case GLFW_KEY_M: key_code = ZFW_KEY_CODE__M; break;
		case GLFW_KEY_N: key_code = ZFW_KEY_CODE__N; break;
		case GLFW_KEY_O: key_code = ZFW_KEY_CODE__O; break;
		case GLFW_KEY_P: key_code = ZFW_KEY_CODE__P; break;
		case GLFW_KEY_Q: key_code = ZFW_KEY_CODE__Q; break;
		case GLFW_KEY_R: key_code = ZFW_KEY_CODE__R; break;
		case GLFW_KEY_S: key_code = ZFW_KEY_CODE__S; break;
		case GLFW_KEY_T: key_code = ZFW_KEY_CODE__T; break;
		case GLFW_KEY_U: key_code = ZFW_KEY_CODE__U; break;
		case GLFW_KEY_V: key_code = ZFW_KEY_CODE__V; break;
		case GLFW_KEY_W: key_code = ZFW_KEY_CODE__W; break;
		case GLFW_KEY_X: key_code = ZFW_KEY_CODE__X; break;
		case GLFW_KEY_Y: key_code = ZFW_KEY_CODE__Y; break;
		case GLFW_KEY_Z: key_code = ZFW_KEY_CODE__Z; break;

		case GLFW_KEY_ESCAPE: key_code = ZFW_KEY_CODE__ESCAPE; break;
		case GLFW_KEY_ENTER: key_code = ZFW_KEY_CODE__ENTER; break;
		case GLFW_KEY_TAB: key_code = ZFW_KEY_CODE__TAB; break;

		case GLFW_KEY_RIGHT: key_code = ZFW_KEY_CODE__RIGHT; break;
		case GLFW_KEY_LEFT: key_code = ZFW_KEY_CODE__LEFT; break;
		case GLFW_KEY_DOWN: key_code = ZFW_KEY_CODE__DOWN; break;
		case GLFW_KEY_UP: key_code = ZFW_KEY_CODE__UP; break;

		case GLFW_KEY_F1: key_code = ZFW_KEY_CODE__F1; break;
		case GLFW_KEY_F2: key_code = ZFW_KEY_CODE__F2; break;
		case GLFW_KEY_F3: key_code = ZFW_KEY_CODE__F3; break;
		case GLFW_KEY_F4: key_code = ZFW_KEY_CODE__F4; break;
		case GLFW_KEY_F5: key_code = ZFW_KEY_CODE__F5; break;
		case GLFW_KEY_F6: key_code = ZFW_KEY_CODE__F6; break;
		case GLFW_KEY_F7: key_code = ZFW_KEY_CODE__F7; break;
		case GLFW_KEY_F8: key_code = ZFW_KEY_CODE__F8; break;
		case GLFW_KEY_F9: key_code = ZFW_KEY_CODE__F9; break;
		case GLFW_KEY_F10: key_code = ZFW_KEY_CODE__F10; break;
		case GLFW_KEY_F11: key_code = ZFW_KEY_CODE__F11; break;
		case GLFW_KEY_F12: key_code = ZFW_KEY_CODE__F12; break;

		case GLFW_KEY_LEFT_SHIFT: key_code = ZFW_KEY_CODE__LEFT_SHIFT; break;
		case GLFW_KEY_LEFT_CONTROL: key_code = ZFW_KEY_CODE__LEFT_CONTROL; break;
		case GLFW_KEY_LEFT_ALT: key_code = ZFW_KEY_CODE__LEFT_ALT; break;
	}

	if (key_code != -1)
	{
		glfw_window_callback_data_t *const callback_data = glfwGetWindowUserPointer(glfw_window);
		const zfw_keys_down_bits_t bitmask = (zfw_keys_down_bits_t)1 << key_code;

		if (glfw_action == GLFW_PRESS)
		{
			callback_data->input_state->keys_down_bits |= bitmask;
		}
		else
		{
			callback_data->input_state->keys_down_bits &= ~bitmask;
		}
	}
}

static void glfw_mouse_button_callback(GLFWwindow *glfw_window, int glfw_button_index, int glfw_action, int glfw_mods)
{
	glfw_window_callback_data_t *const callback_data = glfwGetWindowUserPointer(glfw_window);
	const zfw_mouse_buttons_down_bits_t bitmask = (zfw_mouse_buttons_down_bits_t)1 << glfw_button_index;

	if (glfw_action == GLFW_PRESS)
	{
		callback_data->input_state->mouse_buttons_down_bits |= bitmask;
	}
	else
	{
		callback_data->input_state->mouse_buttons_down_bits &= ~bitmask;
	}
}

static void glfw_mouse_scroll_callback(GLFWwindow *glfw_window, double x_offs, double y_offs)
{
	glfw_window_callback_data_t *const callback_data = glfwGetWindowUserPointer(glfw_window);
	callback_data->input_state->mouse_scroll = (int)y_offs;
}

static void glfw_mouse_pos_callback(GLFWwindow *glfw_window, double mouse_pos_x, double mouse_pos_y)
{
	glfw_window_callback_data_t *const callback_data = glfwGetWindowUserPointer(glfw_window);
	callback_data->input_state->mouse_pos.x = (float)mouse_pos_x;
	callback_data->input_state->mouse_pos.y = (float)mouse_pos_y;
}

static void glfw_joystick_callback(int glfw_joystick_index, int glfw_event)
{
	glfw_window_callback_data_t *const callback_data = glfwGetWindowUserPointer(glfwGetCurrentContext());

	if (glfw_event == GLFW_DISCONNECTED && callback_data->input_state->gamepad_glfw_joystick_index == glfw_joystick_index)
	{
		zfw_reset_gamepad_state(callback_data->input_state);
		zfw_log("Gamepad with GLFW joystick index %d was disconnected.", glfw_joystick_index);
	}
	else if (glfw_event == GLFW_CONNECTED && callback_data->input_state->gamepad_glfw_joystick_index == -1 && glfwJoystickIsGamepad(glfw_joystick_index))
	{
		zfw_reset_gamepad_state(callback_data->input_state);
		callback_data->input_state->gamepad_glfw_joystick_index = glfw_joystick_index;

		zfw_log("Connected gamepad with GLFW joystick index %d.", glfw_joystick_index);
	}
}

////// Static Asset Functions //////
static zfw_bool_t retrieve_user_asset_data_from_assets_file(zfw_user_asset_data_t *const user_asset_data, FILE *const assets_file_fs)
{
	// Retrieve texture data.
	fread(&user_asset_data->tex_data.tex_count, sizeof(user_asset_data->tex_data.tex_count), 1, assets_file_fs);

	if (user_asset_data->tex_data.tex_count)
	{
		user_asset_data->tex_data.gl_ids = zfw_mem_arena_alloc(&zfw_g_main_mem_arena, sizeof(*user_asset_data->tex_data.gl_ids) * user_asset_data->tex_data.tex_count);

		if (!user_asset_data->tex_data.gl_ids)
		{
			zfw_log_error("Failed to allocate %d bytes for texture OpenGL IDs!", sizeof(*user_asset_data->tex_data.gl_ids) * user_asset_data->tex_data.tex_count);
			return ZFW_FALSE;
		}

		glGenTextures(user_asset_data->tex_data.tex_count, user_asset_data->tex_data.gl_ids);

		user_asset_data->tex_data.sizes = zfw_mem_arena_alloc(&zfw_g_main_mem_arena, sizeof(*user_asset_data->tex_data.sizes) * user_asset_data->tex_data.tex_count);

		if (!user_asset_data->tex_data.sizes)
		{
			zfw_log_error("Failed to allocate %d bytes for texture sizes!", sizeof(*user_asset_data->tex_data.sizes) * user_asset_data->tex_data.tex_count);
			return ZFW_FALSE;
		}

		for (int i = 0; i < user_asset_data->tex_data.tex_count; i++)
		{
			fread(&user_asset_data->tex_data.sizes[i], sizeof(user_asset_data->tex_data.sizes[i]), 1, assets_file_fs);

			const int px_data_size = user_asset_data->tex_data.sizes[i].x * user_asset_data->tex_data.sizes[i].y * ZFW_TEX_CHANNEL_COUNT;
			unsigned char *const px_data = zfw_mem_arena_alloc(&zfw_g_main_mem_arena, px_data_size);

			if (!px_data)
			{
				zfw_log_error("Failed to allocate %d bytes for pixel data of texture with ID %d.", px_data_size, i);
				return ZFW_FALSE;
			}

			fread(px_data, sizeof(*px_data), px_data_size / sizeof(*px_data), assets_file_fs);

			glBindTexture(GL_TEXTURE_2D, user_asset_data->tex_data.gl_ids[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, user_asset_data->tex_data.sizes[i].x, user_asset_data->tex_data.sizes[i].y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data);

			// The pixel data for this texture is no longer needed in the arena, so rewind and allow it to be overwritten.
			zfw_rewind_mem_arena(&zfw_g_main_mem_arena);
		}
	}

	// Retrieve shader program data.
	fread(&user_asset_data->shader_prog_data.prog_count, sizeof(user_asset_data->shader_prog_data.prog_count), 1, assets_file_fs);

	if (user_asset_data->shader_prog_data.prog_count)
	{
		user_asset_data->shader_prog_data.gl_ids = zfw_mem_arena_alloc(&zfw_g_main_mem_arena, sizeof(*user_asset_data->shader_prog_data.gl_ids) * user_asset_data->shader_prog_data.prog_count);

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

	return ZFW_TRUE;
}

////// Static Rendering Functions //////
static zfw_bool_t init_sprite_batch_data(zfw_sprite_batch_data_t *const batch_data)
{
	glGenVertexArrays(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_data->vert_array_gl_ids);
	glGenBuffers(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_data->vert_buf_gl_ids);
	glGenBuffers(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_data->elem_buf_gl_ids);

	if (!zfw_init_bitset(&batch_data->slot_activity_bitset, (ZFW_SPRITE_BATCH_SLOT_LIMIT * ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT) / 8))
	{
		zfw_log_error("Failed to initialize a sprite batch slot activity bitset!");
		return ZFW_FALSE;
	}

	return ZFW_TRUE;
}

static void draw_sprite_batches(zfw_sprite_batch_data_t *const batch_data, const zfw_user_tex_data_t *const user_tex_data, const zfw_builtin_shader_prog_data_t *const builtin_shader_prog_data)
{
	for (int i = 0; i < ZFW_RENDER_LAYER_LIMIT; i++)
	{
		if (!batch_data->batch_activity_bits[i])
		{
			// All the batches of this layer are inactive, so continue to the next one.
			continue;
		}

		for (int j = 0; j < ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT; j++)
		{
			if (!(batch_data->batch_activity_bits[i] & ((zfw_render_layer_sprite_batch_activity_bits_t)1 << j)))
			{
				continue;
			}

			int tex_units[ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT] = { 0 };

			for (int k = 0; k < ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT; k++)
			{
				const int user_tex_index = batch_data->tex_units[i][j][k].user_tex_index;

				if (user_tex_index != -1)
				{
					tex_units[k] = k;

					glActiveTexture(GL_TEXTURE0 + k);
					glBindTexture(GL_TEXTURE_2D, user_tex_data->gl_ids[user_tex_index]);
				}
			}

			glUniform1iv(glGetUniformLocation(builtin_shader_prog_data->textured_rect_prog_gl_id, "u_textures"), ZFW_STATIC_ARRAY_LEN(tex_units), tex_units);

			glBindVertexArray(batch_data->vert_array_gl_ids[i][j]);
			glDrawElements(GL_TRIANGLES, 6 * ZFW_SPRITE_BATCH_SLOT_LIMIT, GL_UNSIGNED_SHORT, NULL);
		}
	}
}

static void clean_sprite_batch_data(zfw_sprite_batch_data_t *const batch_data)
{
	zfw_clean_bitset(&batch_data->slot_activity_bitset);

	glDeleteBuffers(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_data->elem_buf_gl_ids);
	glDeleteBuffers(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_data->vert_buf_gl_ids);
	glDeleteVertexArrays(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_data->vert_array_gl_ids);
}

////// Static Game Functions //////
static void clean_game(game_cleanup_data_t *const cleanup_data)
{
	zfw_log("Cleaning up...");

	// Clean sprite batch data.
	for (int i = 0; i < cleanup_data->sprite_batch_datas_cleanup_count; i++)
	{
		clean_sprite_batch_data(&cleanup_data->sprite_batch_datas[i]);
	}

	// Clean built-in shader program data.
	if (cleanup_data->builtin_shader_prog_data)
	{
		glDeleteProgram(cleanup_data->builtin_shader_prog_data->textured_rect_prog_gl_id);
	}

	// Clean user asset data.
	if (cleanup_data->user_asset_data)
	{
		if (cleanup_data->user_asset_data->shader_prog_data.gl_ids)
		{
			for (int i = 0; i < cleanup_data->user_asset_data->shader_prog_data.prog_count; i++)
			{
				glDeleteProgram(cleanup_data->user_asset_data->shader_prog_data.gl_ids[i]);
			}
		}

		if (cleanup_data->user_asset_data->tex_data.gl_ids)
		{
			glDeleteTextures(cleanup_data->user_asset_data->tex_data.tex_count, cleanup_data->user_asset_data->tex_data.gl_ids);
		}
	}

	// Uninitialize GLFW.
	if (cleanup_data->glfw_window)
	{
		glfwDestroyWindow(cleanup_data->glfw_window);
	}

	glfwTerminate();

	// Clean memory arenas.
	if (zfw_g_tick_mem_arena.buf)
	{
		zfw_clean_mem_arena(&zfw_g_tick_mem_arena);
	}

	if (zfw_g_main_mem_arena.buf)
	{
		zfw_clean_mem_arena(&zfw_g_main_mem_arena);
	}
}
///////////////////////////////////

zfw_bool_t zfw_run_game(const zfw_user_game_run_info_t *const user_run_info)
{
	// Ensure data type sizes meet requirements before proceeding.
	if (!zfw_check_data_type_sizes())
	{
		user_run_info->on_clean_func(user_run_info->user_ptr);
		return ZFW_FALSE;
	}

	zfw_log("Data type sizes meet requirements.");

	// Initialize the random number generator.
	srand(time(NULL));
	zfw_log("Successfully initialized the random number generator.");

	// Create and zero-out the game cleanup data struct.
	game_cleanup_data_t cleanup_data = { 0 };

	// Initialize the main memory arena.
	if (!zfw_init_mem_arena(&zfw_g_main_mem_arena, ZFW_MAIN_MEM_ARENA_SIZE))
	{
		zfw_log_error("Failed to initialize the main memory arena! (Size: %d bytes)", ZFW_MAIN_MEM_ARENA_SIZE);

		user_run_info->on_clean_func(user_run_info->user_ptr);
		clean_game(&cleanup_data);

		return ZFW_FALSE;
	}

	// Initialize the tick memory arena.
	if (!zfw_init_mem_arena(&zfw_g_tick_mem_arena, ZFW_TICK_MEM_ARENA_SIZE))
	{
		zfw_log_error("Failed to initialize the tick memory arena! (Size: %d bytes)", ZFW_TICK_MEM_ARENA_SIZE);

		user_run_info->on_clean_func(user_run_info->user_ptr);
		clean_game(&cleanup_data);

		return ZFW_FALSE;
	}

	// Initialize GLFW.
	if (!glfwInit())
	{
		zfw_log_error("Failed to initialize GLFW!");

		user_run_info->on_clean_func(user_run_info->user_ptr);
		clean_game(&cleanup_data);

		return ZFW_FALSE;
	}

	zfw_log("Successfully initialized GLFW!");

	// Set up the GLFW window.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	GLFWwindow *const glfw_window = glfwCreateWindow(user_run_info->init_window_size.x, user_run_info->init_window_size.y, user_run_info->init_window_title, NULL, NULL);

	if (!glfw_window)
	{
		zfw_log_error("Failed to create a GLFW window!");

		user_run_info->on_clean_func(user_run_info->user_ptr);
		clean_game(&cleanup_data);

		return ZFW_FALSE;
	}

	zfw_log("Successfully created a GLFW window!");

	cleanup_data.glfw_window = glfw_window;

	glfwMakeContextCurrent(glfw_window);

	// Initialize the window state.
	zfw_window_state_t window_state;
	glfwGetWindowSize(glfw_window, &window_state.size.x, &window_state.size.y);
	glfwGetWindowPos(glfw_window, &window_state.pos.x, &window_state.pos.y);
	window_state.fullscreen = ZFW_FALSE;

	zfw_bool_t user_window_fullscreen = user_run_info->init_window_fullscreen; // Whether or not the user wants the window to be in fullscreen, assignable by them through pointers passed into their game functions. The actual state will not be updated until a specific point in the main loop.

	// Initialize the input state.
	zfw_input_state_t input_state = { 0 };

	input_state.gamepad_glfw_joystick_index = -1;

	for (int i = 0; i <= GLFW_JOYSTICK_LAST; i++)
	{
		if (glfwJoystickIsGamepad(i))
		{
			input_state.gamepad_glfw_joystick_index = i;
			zfw_log("Connected gamepad with GLFW joystick index %d on initialization.", i);
			break;
		}
	}

	// Set GLFW callbacks.
	glfw_window_callback_data_t glfw_window_callback_data = { &window_state, &input_state };
	glfwSetWindowUserPointer(glfw_window, &glfw_window_callback_data);

	glfwSetWindowSizeCallback(glfw_window, glfw_window_size_callback);
	glfwSetWindowPosCallback(glfw_window, glfw_window_pos_callback);

	glfwSetKeyCallback(glfw_window, glfw_key_callback);
	glfwSetMouseButtonCallback(glfw_window, glfw_mouse_button_callback);
	glfwSetScrollCallback(glfw_window, glfw_mouse_scroll_callback);
	glfwSetCursorPosCallback(glfw_window, glfw_mouse_pos_callback);
	glfwSetJoystickCallback(glfw_joystick_callback);

	// Set up OpenGL function pointers.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		zfw_log_error("Failed to initialize OpenGL function pointers!");

		user_run_info->on_clean_func(user_run_info->user_ptr);
		clean_game(&cleanup_data);

		return ZFW_FALSE;
	}

	zfw_log("Successfully set up OpenGL function pointers!");

	// Initialize user asset data.
	zfw_user_asset_data_t user_asset_data = { 0 };

	{
		FILE *const assets_file_fs = fopen(ZFW_ASSETS_FILE_NAME, "rb");

		if (!assets_file_fs)
		{
			zfw_log_error("Failed to open assets file \"%s\"!", ZFW_ASSETS_FILE_NAME);

			user_run_info->on_clean_func(user_run_info->user_ptr);
			clean_game(&cleanup_data);

			return ZFW_FALSE;
		}

		const zfw_bool_t asset_data_read_successful = retrieve_user_asset_data_from_assets_file(&user_asset_data, assets_file_fs);

		fclose(assets_file_fs);

		if (!asset_data_read_successful)
		{
			user_run_info->on_clean_func(user_run_info->user_ptr);
			clean_game(&cleanup_data);

			return ZFW_FALSE;
		}

		cleanup_data.user_asset_data = &user_asset_data;
	}

	// Initialize built-in shader programs.
	zfw_builtin_shader_prog_data_t builtin_shader_prog_data;
	zfw_gen_shader_prog(&builtin_shader_prog_data.textured_rect_prog_gl_id, ZFW_BUILTIN_TEXTURED_RECT_VERT_SHADER_SRC, ZFW_BUILTIN_TEXTURED_RECT_FRAG_SHADER_SRC);

	cleanup_data.builtin_shader_prog_data = &builtin_shader_prog_data;

	// Set up rendering.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	zfw_sprite_batch_data_t sprite_batch_datas[ZFW_SPRITE_BATCH_DATA_ID_COUNT];
	cleanup_data.sprite_batch_datas = sprite_batch_datas;

	for (int i = 0; i < ZFW_SPRITE_BATCH_DATA_ID_COUNT; i++)
	{
		if (!init_sprite_batch_data(&sprite_batch_datas[i]))
		{
			user_run_info->on_clean_func(user_run_info->user_ptr);
			clean_game(&cleanup_data);

			return ZFW_FALSE;
		}

		cleanup_data.sprite_batch_datas_cleanup_count++;
	}

	// Show the window now that things have been set up.
	glfwShowWindow(glfw_window);
	//

	zfw_input_state_t last_tick_input_state = { 0 }; // This is to be a copy of the input state at the point of the last tick.
	zfw_window_state_t window_prefullscreen_state; // This is to be a copy of the window state prior to switching from windowed mode to fullscreen, so that this state can be returned to when switching back.

	// Perform the restart loop.
	while (ZFW_TRUE)
	{
		zfw_bool_t restart = ZFW_FALSE; // Whether or not to break out of the main loop and restart the game. This is modifiable by the user in their tick function through a pointer.

		for (int i = 0; i < ZFW_SPRITE_BATCH_DATA_ID_COUNT; i++)
		{
			memset(sprite_batch_datas[i].batch_activity_bits, 0, sizeof(sprite_batch_datas[i].batch_activity_bits));
			zfw_clear_bitset(&sprite_batch_datas[i].slot_activity_bitset);
		}

		zfw_view_state_t view_state = { 0 };
		view_state.scale = 1.0f;

		// Run the user-defined game initialization function.
		{
			zfw_user_game_init_func_data_t init_data;
			init_data.window_size = window_state.size;
			init_data.window_fullscreen = &user_window_fullscreen;
			init_data.user_asset_data = &user_asset_data;
			init_data.sprite_batch_datas = sprite_batch_datas;
			init_data.view_state = &view_state;

			user_run_info->on_init_func(user_run_info->user_ptr, &init_data);
		}

		// Perform the main loop.
		double frame_time_last = glfwGetTime();

		const int target_ticks_per_sec = 60;
		const double target_tick_interval = 1.0 / target_ticks_per_sec;
		double frame_time_interval_accum = target_tick_interval; // (The assignment here ensures that a tick is always run on the first frame.)

		zfw_bool_t glfw_window_should_close;

		while (!(glfw_window_should_close = glfwWindowShouldClose(glfw_window)) && !restart)
		{
			const zfw_vec_2d_i_t window_size_last = window_state.size;

			glfwPollEvents();

			// If the user has defined a window resize function and the window has been resized, call the function.
			if (user_run_info->on_window_resize_func && (window_state.size.x != window_size_last.x || window_state.size.y != window_size_last.y))
			{
				zfw_user_window_resize_func_data_t window_resize_data;
				window_resize_data.window_size = window_state.size;
				window_resize_data.user_asset_data = &user_asset_data;
				window_resize_data.sprite_batch_datas = sprite_batch_datas;
				window_resize_data.view_state = &view_state;

				user_run_info->on_window_resize_func(user_run_info->user_ptr, &window_resize_data);
			}

			// Refresh the gamepad state.
			if (input_state.gamepad_glfw_joystick_index != -1)
			{
				GLFWgamepadstate glfw_gamepad_state;

				if (glfwGetGamepadState(input_state.gamepad_glfw_joystick_index, &glfw_gamepad_state))
				{
					input_state.gamepad_buttons_down_bits = 0;

					for (int i = 0; i < ZFW_GAMEPAD_BUTTON_CODE_COUNT; i++)
					{
						input_state.gamepad_buttons_down_bits |= (zfw_gamepad_buttons_down_bits_t)(glfw_gamepad_state.buttons[i] == GLFW_PRESS) << i;
					}

					for (int i = 0; i < ZFW_GAMEPAD_AXIS_CODE_COUNT; i++)
					{
						input_state.gamepad_axis_values[i] = glfw_gamepad_state.axes[i];
					}
				}
				else
				{
					zfw_log_error("Failed to retrieve the state of gamepad with GLFW joystick index %d.", input_state.gamepad_glfw_joystick_index);
					zfw_reset_gamepad_state(&input_state);
				}
			}
			//

			const double frame_time = glfwGetTime();
			frame_time_interval_accum += frame_time - frame_time_last;
			frame_time_last = frame_time;

			int tick_count = (int)(frame_time_interval_accum / target_tick_interval);

			for (int i = 0; i < tick_count; i++)
			{
				{
					// Run user-defined game tick function.
					zfw_user_game_tick_func_data_t tick_data;
					tick_data.restart = &restart;
					tick_data.window_size = window_state.size;
					tick_data.window_fullscreen = &user_window_fullscreen;
					tick_data.input_state = &input_state;
					tick_data.input_state_last = &last_tick_input_state;
					tick_data.user_asset_data = &user_asset_data;
					tick_data.sprite_batch_datas = sprite_batch_datas;
					tick_data.view_state = &view_state;

					user_run_info->on_tick_func(user_run_info->user_ptr, &tick_data);
				}

				zfw_reset_mem_arena(&zfw_g_tick_mem_arena);

				frame_time_interval_accum -= target_tick_interval;

				if (i == tick_count - 1)
				{
					// Reset the mouse scroll now that the ticks are complete.
					input_state.mouse_scroll = 0;

					// Update the tick input state for next time.
					last_tick_input_state = input_state;
				}
			}

			// If the user has requested a change to the fullscreen state, update it.
			if (window_state.fullscreen != user_window_fullscreen)
			{
				zfw_bool_t call_user_window_resize_func = ZFW_TRUE;

				if (user_window_fullscreen)
				{
					GLFWmonitor *glfw_primary_monitor = glfwGetPrimaryMonitor();

					if (glfw_primary_monitor)
					{
						window_prefullscreen_state = window_state;

						const GLFWvidmode *glfw_video_mode = glfwGetVideoMode(glfw_primary_monitor);
						glfwSetWindowMonitor(glfw_window, glfw_primary_monitor, 0, 0, glfw_video_mode->width, glfw_video_mode->height, glfw_video_mode->refreshRate);
					}
					else
					{
						zfw_log_error("Failed to switch to fullscreen as GLFW could not find a primary monitor.");
						call_user_window_resize_func = ZFW_FALSE;
					}
				}
				else
				{
					glfwSetWindowMonitor(glfw_window, NULL, window_prefullscreen_state.pos.x, window_prefullscreen_state.pos.y, window_prefullscreen_state.size.x, window_prefullscreen_state.size.y, 0);
				}

				window_state.fullscreen = user_window_fullscreen;

				if (user_run_info->on_window_resize_func && call_user_window_resize_func)
				{
					zfw_user_window_resize_func_data_t window_resize_data;
					window_resize_data.window_size = window_state.size;
					window_resize_data.user_asset_data = &user_asset_data;
					window_resize_data.sprite_batch_datas = sprite_batch_datas;
					window_resize_data.view_state = &view_state;

					user_run_info->on_window_resize_func(user_run_info->user_ptr, &window_resize_data);
				}
			}

			// Rendering
			if (tick_count > 0)
			{
				glClearColor(0.2f, 0.075f, 0.15f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);

				{
					glUseProgram(builtin_shader_prog_data.textured_rect_prog_gl_id);

					zfw_matrix_4x4_t view;
					const int view_uni_location = glGetUniformLocation(builtin_shader_prog_data.textured_rect_prog_gl_id, "u_view");

					view.elems[0][0] = view_state.scale;
					view.elems[1][1] = view_state.scale;
					view.elems[2][2] = 1.0f;
					view.elems[3][0] = -view_state.pos.x * view_state.scale;
					view.elems[3][1] = -view_state.pos.y * view_state.scale;
					glUniformMatrix4fv(view_uni_location, 1, GL_FALSE, (GLfloat *)&view.elems);

					zfw_matrix_4x4_t proj;
					zfw_init_ortho_matrix_4x4(&proj, 0.0f, window_state.size.x, window_state.size.y, 0.0f, -1.0f, 1.0f);
					glUniformMatrix4fv(glGetUniformLocation(builtin_shader_prog_data.textured_rect_prog_gl_id, "u_proj"), 1, GL_FALSE, (GLfloat *)&proj.elems);

					draw_sprite_batches(&sprite_batch_datas[ZFW_SPRITE_BATCH_DATA_ID__VIEW], &user_asset_data.tex_data, &builtin_shader_prog_data);

					zfw_init_identity_matrix_4x4(&view);
					glUniformMatrix4fv(view_uni_location, 1, GL_FALSE, (GLfloat *)&view.elems);

					draw_sprite_batches(&sprite_batch_datas[ZFW_SPRITE_BATCH_DATA_ID__SCREEN], &user_asset_data.tex_data, &builtin_shader_prog_data);
				}

				glfwSwapBuffers(glfw_window);
			}
		}
		//

		if (!restart || glfw_window_should_close)
		{
			break;
		}

		zfw_log("Restarting...");
	}
	//

	user_run_info->on_clean_func(user_run_info->user_ptr);
	clean_game(&cleanup_data);

	return ZFW_TRUE;
}
