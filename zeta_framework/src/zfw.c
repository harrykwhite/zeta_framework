#include "zfw.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// DESCRIPTION: Pointers to data to be cleaned up when the game ends.
typedef struct
{
	GLFWwindow *glfw_window;

	zfw_user_asset_data_t *user_asset_data;
	zfw_builtin_shader_prog_data_t *builtin_shader_prog_data;

	zfw_sprite_batch_data_t *view_sprite_batch_data;
	zfw_sprite_batch_data_t *screen_sprite_batch_data;
} zfw_game_cleanup_info_t;

// DESCRIPTION: Window and input data intended for access through a user pointer in GLFW callback functions.
typedef struct
{
	zfw_window_state_t window_state;
	zfw_input_state_t input_state;
} zfw_glfw_window_user_data_t;

static void clean_game(zfw_game_cleanup_info_t *const cleanup_info);

static void glfw_window_size_callback(GLFWwindow *glfw_window, int width, int height);
static void glfw_window_pos_callback(GLFWwindow *glfw_window, int x, int y);
static void glfw_key_callback(GLFWwindow *glfw_window, int glfw_key_index, int glfw_scancode, int glfw_action, int glfw_mods);
static void glfw_mouse_button_callback(GLFWwindow *glfw_window, int glfw_button_index, int glfw_action, int glfw_mods);
static void glfw_mouse_pos_callback(GLFWwindow *glfw_window, double mouse_pos_x, double mouse_pos_y);
static void glfw_joystick_callback(int glfw_joystick_index, int glfw_event);

static void reset_gamepad_state(zfw_input_state_t *const input_state);

static void init_sprite_batch_data(zfw_sprite_batch_data_t *const batch_data);
static zfw_bool_t init_and_activate_render_layer_sprite_batch(const int layer_index, const int batch_index, zfw_sprite_batch_data_t *const batch_data);
static void draw_sprite_batches(zfw_sprite_batch_data_t *const batch_data, const zfw_user_tex_data_t *const user_tex_data, const zfw_builtin_shader_prog_data_t *const builtin_shader_prog_data);
static void clean_sprite_batch_data(zfw_sprite_batch_data_t *const batch_data);

zfw_bool_t zfw_run_game(const zfw_game_run_info_t *const run_info)
{
	// TODO: Verify that certain things in the game run information struct are set.

	// Initialize the game cleanup information struct.
	zfw_game_cleanup_info_t cleanup_info = { 0 };

	// Ensure data type sizes meet requirements before proceeding.
	if (!zfw_check_data_type_sizes())
	{
		run_info->on_clean_func(run_info->user_ptr);
		clean_game(&cleanup_info);

		return ZFW_FALSE;
	}

	zfw_log("Data type sizes meet requirements.");

	// Initialize GLFW.
	if (!glfwInit())
	{
		zfw_log_error("Failed to initialize GLFW!");

		run_info->on_clean_func(run_info->user_ptr);
		clean_game(&cleanup_info);

		return ZFW_FALSE;
	}

	zfw_log("Successfully initialized GLFW!");

	// Set up the GLFW window.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	GLFWwindow *glfw_window = glfwCreateWindow(run_info->init_window_size.x, run_info->init_window_size.y, run_info->init_window_title, NULL, NULL);
	cleanup_info.glfw_window = glfw_window;

	if (!glfw_window)
	{
		zfw_log_error("Failed to create a GLFW window!");

		run_info->on_clean_func(run_info->user_ptr);
		clean_game(&cleanup_info);

		return ZFW_FALSE;
	}

	zfw_log("Successfully created a GLFW window!");

	glfwMakeContextCurrent(glfw_window);
	//

	zfw_glfw_window_user_data_t glfw_window_user_data;

	// Initialize the window state.
	glfwGetWindowSize(glfw_window, &glfw_window_user_data.window_state.size.x, &glfw_window_user_data.window_state.size.y);
	glfwGetWindowPos(glfw_window, &glfw_window_user_data.window_state.pos.x, &glfw_window_user_data.window_state.pos.y);

	// Initialize the input state.
	memset(&glfw_window_user_data.input_state, 0, sizeof(glfw_window_user_data.input_state));

	glfw_window_user_data.input_state.gamepad_glfw_joystick_index = -1;

	for (int i = 0; i <= GLFW_JOYSTICK_LAST; i++)
	{
		if (glfwJoystickIsGamepad(i))
		{
			glfw_window_user_data.input_state.gamepad_glfw_joystick_index = i;
			zfw_log("Connected gamepad with GLFW joystick index %d on initialization.", glfw_window_user_data.input_state.gamepad_glfw_joystick_index);
			break;
		}
	}

	// Set GLFW callbacks.
	glfwSetWindowUserPointer(glfw_window, &glfw_window_user_data);

	glfwSetWindowSizeCallback(glfw_window, glfw_window_size_callback);
	glfwSetWindowPosCallback(glfw_window, glfw_window_pos_callback);

	glfwSetKeyCallback(glfw_window, glfw_key_callback);
	glfwSetMouseButtonCallback(glfw_window, glfw_mouse_button_callback);
	glfwSetCursorPosCallback(glfw_window, glfw_mouse_pos_callback);
	glfwSetJoystickCallback(glfw_joystick_callback);

	// Set up OpenGL function pointers.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		zfw_log_error("Failed to initialize OpenGL function pointers!");

		run_info->on_clean_func(run_info->user_ptr);
		clean_game(&cleanup_info);

		return ZFW_FALSE;
	}

	zfw_log("Successfully set up OpenGL function pointers!");

	// Initialize user asset data.
	zfw_user_asset_data_t user_asset_data = { 0 };
	cleanup_info.user_asset_data = &user_asset_data;

	{
		// Open the assets file.
		FILE *const assets_file_fs = fopen(ZFW_ASSETS_FILE_NAME, "rb");

		if (!assets_file_fs)
		{
			zfw_log_error("Failed to open assets file \"%s\"!", ZFW_ASSETS_FILE_NAME);

			run_info->on_clean_func(run_info->user_ptr);
			clean_game(&cleanup_info);

			return ZFW_FALSE;
		}

		// Read texture data.
		fread(&user_asset_data.tex_data.tex_count, sizeof(user_asset_data.tex_data.tex_count), 1, assets_file_fs);

		if (user_asset_data.tex_data.tex_count)
		{
			user_asset_data.tex_data.gl_ids = malloc(sizeof(*user_asset_data.tex_data.gl_ids) * user_asset_data.tex_data.tex_count);

			if (!user_asset_data.tex_data.gl_ids)
			{
				zfw_log_error("Failed to allocate %d bytes for texture OpenGL IDs!", sizeof(*user_asset_data.tex_data.gl_ids) * user_asset_data.tex_data.tex_count);

				run_info->on_clean_func(run_info->user_ptr);
				clean_game(&cleanup_info);

				return ZFW_FALSE;
			}

			user_asset_data.tex_data.sizes = malloc(sizeof(*user_asset_data.tex_data.sizes) * user_asset_data.tex_data.tex_count);

			if (!user_asset_data.tex_data.sizes)
			{
				zfw_log_error("Failed to allocate %d bytes for texture sizes!", sizeof(*user_asset_data.tex_data.sizes) * user_asset_data.tex_data.tex_count);

				run_info->on_clean_func(run_info->user_ptr);
				clean_game(&cleanup_info);

				return ZFW_FALSE;
			}

			glGenTextures(user_asset_data.tex_data.tex_count, user_asset_data.tex_data.gl_ids);

			for (int i = 0; i < user_asset_data.tex_data.tex_count; i++)
			{
				fread(&user_asset_data.tex_data.sizes[i], sizeof(user_asset_data.tex_data.sizes[i]), 1, assets_file_fs);

				const int px_data_size = user_asset_data.tex_data.sizes[i].x * user_asset_data.tex_data.sizes[i].y * ZFW_TEX_CHANNEL_COUNT;
				unsigned char *const px_data = malloc(px_data_size);

				if (!px_data)
				{
					zfw_log_error("Failed to allocate %d bytes for pixel data of texture with ID %d.", px_data_size, i);

					run_info->on_clean_func(run_info->user_ptr);
					clean_game(&cleanup_info);

					return ZFW_FALSE;
				}

				fread(px_data, sizeof(*px_data), px_data_size / sizeof(*px_data), assets_file_fs);

				glBindTexture(GL_TEXTURE_2D, user_asset_data.tex_data.gl_ids[i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, user_asset_data.tex_data.sizes[i].x, user_asset_data.tex_data.sizes[i].y, 0, GL_RGBA, GL_UNSIGNED_BYTE, px_data);
			}
		}

		// Read shader program data.
		fread(&user_asset_data.shader_prog_data.prog_count, sizeof(user_asset_data.shader_prog_data.prog_count), 1, assets_file_fs);

		if (user_asset_data.shader_prog_data.prog_count)
		{
			user_asset_data.shader_prog_data.gl_ids = malloc(sizeof(*user_asset_data.shader_prog_data.gl_ids) * user_asset_data.shader_prog_data.prog_count);

			if (!user_asset_data.shader_prog_data.gl_ids)
			{
				zfw_log_error("Failed to allocate %d bytes for shader program OpenGL IDs!", sizeof(*user_asset_data.shader_prog_data.gl_ids) * user_asset_data.shader_prog_data.prog_count);

				run_info->on_clean_func(run_info->user_ptr);
				clean_game(&cleanup_info);

				return ZFW_FALSE;
			}

			for (int i = 0; i < user_asset_data.shader_prog_data.prog_count; i++)
			{
				char vert_shader_src_buf[ZFW_SHADER_SRC_MAX_LEN];
				fread(vert_shader_src_buf, sizeof(*vert_shader_src_buf), sizeof(vert_shader_src_buf) / sizeof(*vert_shader_src_buf), assets_file_fs);

				char frag_shader_src_buf[ZFW_SHADER_SRC_MAX_LEN];
				fread(frag_shader_src_buf, sizeof(*frag_shader_src_buf), sizeof(frag_shader_src_buf) / sizeof(*frag_shader_src_buf), assets_file_fs);

				zfw_set_up_shader_prog(&user_asset_data.shader_prog_data.gl_ids[i], vert_shader_src_buf, frag_shader_src_buf);
			}
		}
	}

	// Initialize built-in shader programs.
	zfw_builtin_shader_prog_data_t builtin_shader_prog_data;
	zfw_set_up_shader_prog(&builtin_shader_prog_data.textured_rect_prog_gl_id, ZFW_BUILTIN_TEXTURED_RECT_VERT_SHADER_SRC, ZFW_BUILTIN_TEXTURED_RECT_FRAG_SHADER_SRC);

	// Set up rendering.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	zfw_sprite_batch_data_t view_sprite_batch_data;
	cleanup_info.view_sprite_batch_data = &view_sprite_batch_data;
	init_sprite_batch_data(&view_sprite_batch_data);

	zfw_sprite_batch_data_t screen_sprite_batch_data;
	cleanup_info.screen_sprite_batch_data = &screen_sprite_batch_data;
	init_sprite_batch_data(&screen_sprite_batch_data);

	zfw_view_state_t view_state = { zfw_create_vec_2d(0.0f, 0.0f), 1.0f };

	// Run user-defined game initialization function.
	{
		zfw_user_game_init_func_data_t init_data;
		init_data.window_size = glfw_window_user_data.window_state.size;
		init_data.user_asset_data = &user_asset_data;
		init_data.builtin_shader_prog_data = &builtin_shader_prog_data;
		init_data.view_sprite_batch_data = &view_sprite_batch_data;
		init_data.screen_sprite_batch_data = &screen_sprite_batch_data;
		init_data.view_state = &view_state;

		run_info->on_init_func(run_info->user_ptr, &init_data);
	}

	// Show the window now that things have been set up.
	glfwShowWindow(glfw_window);

	// Main Loop
	int windowed = ZFW_TRUE;
	zfw_window_state_t window_state_windowed;

	const int target_ticks_per_sec = 60;
	const double target_tick_dur = 1.0 / target_ticks_per_sec;
	double tick_dur_accum = 0.0;
	double frame_time_last = glfwGetTime();

	zfw_input_state_t frame_input_state = { 0 };

	while (!glfwWindowShouldClose(glfw_window))
	{
		const zfw_vec_2d_i_t window_size_last = glfw_window_user_data.window_state.size;

		glfwPollEvents();

		// If the window has been resized, call user-defined window resize function.
		if ((glfw_window_user_data.window_state.size.x != window_size_last.x || glfw_window_user_data.window_state.size.y != window_size_last.y) && run_info->on_window_resize_func)
		{
			zfw_user_window_resize_func_data_t window_resize_data;
			window_resize_data.window_size = glfw_window_user_data.window_state.size;

			run_info->on_window_resize_func(run_info->user_ptr, &window_resize_data);
		}

		// If we are in windowed mode, store the window state so that it can be accessed if later we're in fullscreen mode and want to go back.
		if (windowed)
		{
			window_state_windowed = glfw_window_user_data.window_state;
		}

		// Update the gamepad state.
		if (glfw_window_user_data.input_state.gamepad_glfw_joystick_index != -1)
		{
			GLFWgamepadstate gamepad_state;

			if (glfwGetGamepadState(glfw_window_user_data.input_state.gamepad_glfw_joystick_index, &gamepad_state))
			{
				glfw_window_user_data.input_state.gamepad_buttons_down_bits = 0;

				for (int i = 0; i < ZFW_NUM_GAMEPAD_BUTTON_CODES; i++)
				{
					glfw_window_user_data.input_state.gamepad_buttons_down_bits |= (zfw_gamepad_buttons_down_bits_t)(gamepad_state.buttons[i] == GLFW_PRESS) << i;
				}

				for (int i = 0; i < ZFW_NUM_GAMEPAD_AXIS_CODES; i++)
				{
					glfw_window_user_data.input_state.gamepad_axis_values[i] = gamepad_state.axes[i];
				}
			}
			else
			{
				zfw_log_error("Failed to retrieve the state of gamepad with GLFW joystick index %d.", glfw_window_user_data.input_state.gamepad_glfw_joystick_index);
				reset_gamepad_state(&glfw_window_user_data.input_state);
			}
		}
		//

		const double frame_time = glfwGetTime();
		tick_dur_accum += frame_time - frame_time_last;
		frame_time_last = frame_time;

		const int tick_count = (int)(tick_dur_accum / target_tick_dur);

		if (tick_count > 0)
		{
			const zfw_input_state_t frame_input_state_last = frame_input_state;
			frame_input_state = glfw_window_user_data.input_state;

			const int windowed_last = windowed;

			// Execute ticks.
			for (int i = 0; i < tick_count; i++)
			{
				{
					// Run user-defined game tick function.
					zfw_user_game_tick_func_data_t tick_data;
					tick_data.windowed = &windowed;
					tick_data.window_size = glfw_window_user_data.window_state.size;
					tick_data.input_state = &frame_input_state;
					tick_data.input_state_last = &frame_input_state_last;
					tick_data.user_asset_data = &user_asset_data;
					tick_data.view_sprite_batch_data = &view_sprite_batch_data;
					tick_data.screen_sprite_batch_data = &screen_sprite_batch_data;
					tick_data.view_state = &view_state;

					run_info->on_tick_func(run_info->user_ptr, &tick_data);
				}

				tick_dur_accum -= target_tick_dur;
			}

			// Windowed/Fullscreen Mode Swapping
			if (windowed != windowed_last)
			{
				zfw_bool_t call_user_window_resize_func = ZFW_TRUE;

				if (!windowed)
				{
					GLFWmonitor *glfw_primary_monitor = glfwGetPrimaryMonitor();

					if (glfw_primary_monitor)
					{
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
					glfwSetWindowMonitor(glfw_window, NULL, window_state_windowed.pos.x, window_state_windowed.pos.y, window_state_windowed.size.x, window_state_windowed.size.y, 0);
				}

				if (call_user_window_resize_func && run_info->on_window_resize_func)
				{
					zfw_user_window_resize_func_data_t window_resize_data;
					window_resize_data.window_size = glfw_window_user_data.window_state.size;

					run_info->on_window_resize_func(run_info->user_ptr, &window_resize_data);
				}
			}

			// Rendering
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
				zfw_init_ortho_matrix_4x4(&proj, 0.0f, glfw_window_user_data.window_state.size.x, glfw_window_user_data.window_state.size.y, 0.0f, -1.0f, 1.0f);
				glUniformMatrix4fv(glGetUniformLocation(builtin_shader_prog_data.textured_rect_prog_gl_id, "u_proj"), 1, GL_FALSE, (GLfloat *)&proj.elems);

				draw_sprite_batches(&view_sprite_batch_data, &user_asset_data.tex_data, &builtin_shader_prog_data);

				zfw_init_identity_matrix_4x4(&view);
				glUniformMatrix4fv(view_uni_location, 1, GL_FALSE, (GLfloat *)&view.elems);

				draw_sprite_batches(&screen_sprite_batch_data, &user_asset_data.tex_data, &builtin_shader_prog_data);
			}

			glfwSwapBuffers(glfw_window);
		}
	}

	// Clean up.
	run_info->on_clean_func(run_info->user_ptr);
	clean_game(&cleanup_info);
	//

	return ZFW_TRUE;
}

zfw_bool_t zfw_is_key_down(const zfw_key_code_t key_code, const zfw_input_state_t *const input_state)
{
	return (input_state->keys_down_bits & ((zfw_keys_down_bits_t)1 << key_code)) != 0;
}

zfw_bool_t zfw_is_key_pressed(const zfw_key_code_t key_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
	return zfw_is_key_down(key_code, input_state) && !zfw_is_key_down(key_code, input_state_last);
}

zfw_bool_t zfw_is_key_released(const zfw_key_code_t key_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
	return !zfw_is_key_down(key_code, input_state) && zfw_is_key_down(key_code, input_state_last);
}

zfw_bool_t zfw_is_mouse_button_down(const zfw_mouse_button_code_t button_code, const zfw_input_state_t *const input_state)
{
	return (input_state->mouse_buttons_down_bits & ((zfw_mouse_buttons_down_bits_t)1 << button_code)) != 0;
}

zfw_bool_t zfw_is_mouse_button_pressed(const zfw_mouse_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
	return zfw_is_mouse_button_down(button_code, input_state) && !zfw_is_mouse_button_down(button_code, input_state_last);
}

zfw_bool_t zfw_is_mouse_button_released(const zfw_mouse_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
	return !zfw_is_mouse_button_down(button_code, input_state) && zfw_is_mouse_button_down(button_code, input_state_last);
}

zfw_bool_t zfw_is_gamepad_button_down(const zfw_gamepad_button_code_t button_code, const zfw_input_state_t *const input_state)
{
	return (input_state->gamepad_buttons_down_bits & ((zfw_gamepad_buttons_down_bits_t)1 << button_code)) != 0;
}

zfw_bool_t zfw_is_gamepad_button_pressed(const zfw_gamepad_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
	return zfw_is_gamepad_button_down(button_code, input_state) && !zfw_is_gamepad_button_down(button_code, input_state_last);
}

zfw_bool_t zfw_is_gamepad_button_released(const zfw_gamepad_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
	return !zfw_is_gamepad_button_down(button_code, input_state) && zfw_is_gamepad_button_down(button_code, input_state_last);
}

void zfw_set_up_shader_prog(GLuint *const shader_prog_gl_id, const char *const vert_shader_src, const char *const frag_shader_src)
{
	// Create the vertex shader.
	const GLuint vert_shader_glid = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert_shader_glid, 1, &vert_shader_src, NULL);

	glCompileShader(vert_shader_glid);

	// Create the fragment shader.
	const GLuint frag_shader_glid = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_shader_glid, 1, &frag_shader_src, NULL);

	glCompileShader(frag_shader_glid);

	// Create the shader program using the shaders.
	*shader_prog_gl_id = glCreateProgram();

	glAttachShader(*shader_prog_gl_id, vert_shader_glid);
	glAttachShader(*shader_prog_gl_id, frag_shader_glid);

	glLinkProgram(*shader_prog_gl_id);

	// Delete the shaders as they're no longer needed.
	glDeleteShader(frag_shader_glid);
	glDeleteShader(vert_shader_glid);
}

zfw_bool_t zfw_take_slot_from_render_layer_sprite_batch(const int layer_index, const int user_tex_index, zfw_sprite_batch_slot_key_t *const key, zfw_sprite_batch_data_t *const batch_data)
{
	memset(key, -1, sizeof(*key));

	int first_inactive_batch_index = -1;

	for (int i = 0; i < ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT; i++)
	{
		// Check if the batch is active.
		if (!(batch_data->batch_activities[layer_index] & ((zfw_render_layer_sprite_batch_activity_bits_t)1 << i)))
		{
			if (first_inactive_batch_index == -1)
			{
				first_inactive_batch_index = i;
			}

			continue;
		}

		// Check for a texture unit to use.
		key->batch_tex_unit = -1;

		for (int j = 0; j < ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT; j++)
		{
			if (batch_data->user_tex_indexes[layer_index][i][j] == -1 || batch_data->user_tex_indexes[layer_index][i][j] == user_tex_index)
			{
				key->batch_tex_unit = j;
				break;
			}
		}

		if (key->batch_tex_unit == -1)
		{
			continue;
		}

		// Check for an available slot.
		for (int j = 0; j < ZFW_ARRAY_LEN(batch_data->slot_activity_bits[layer_index][i]); j++)
		{
			if (batch_data->slot_activity_bits[layer_index][i][j] == (zfw_sprite_batch_slot_activity_bits_t)(~0))
			{
				continue;
			}

			for (int k = 0; k < ZFW_SIZE_IN_BITS(zfw_sprite_batch_slot_activity_bits_t); k++)
			{
				const zfw_sprite_batch_slot_activity_bits_t activity_bitmask = (zfw_sprite_batch_slot_activity_bits_t)1 << k;

				if (!(batch_data->slot_activity_bits[layer_index][i][j] & activity_bitmask))
				{
					// Take the batch slot.
					key->batch_index = i;
					key->batch_slot_index = (j * ZFW_SIZE_IN_BITS(zfw_sprite_batch_slot_activity_bits_t)) + k;

					batch_data->user_tex_indexes[layer_index][i][key->batch_tex_unit] = user_tex_index;
					batch_data->slot_activity_bits[layer_index][i][j] |= activity_bitmask;

					return ZFW_TRUE;
				}
			}
		}
	}

	if (first_inactive_batch_index != -1)
	{
		// Initialize and activate a new sprite batch. If successful, try this all again.
		if (init_and_activate_render_layer_sprite_batch(layer_index, first_inactive_batch_index, batch_data)) // WARNING: The slot count is 0 here.
		{
			return zfw_take_slot_from_render_layer_sprite_batch(layer_index, user_tex_index, key, batch_data);
		}
	}

	return ZFW_FALSE;
}

void zfw_write_to_render_layer_sprite_batch_slot(const int layer_index, const zfw_sprite_batch_slot_key_t *const slot_key, const zfw_vec_2d_t pos, const float rot, const zfw_vec_2d_t scale, const zfw_vec_2d_t origin, const float opacity, const zfw_sprite_batch_data_t *const batch_data, const zfw_user_tex_data_t *const user_tex_data)
{
	const zfw_vec_2d_i_t user_tex_size = user_tex_data->sizes[batch_data->user_tex_indexes[layer_index][slot_key->batch_index][slot_key->batch_tex_unit]];

	const float verts[] = {
		(0.0f - origin.x) * scale.x,
		(0.0f - origin.y) * scale.y,
		pos.x,
		pos.y,
		user_tex_size.x,
		user_tex_size.y,
		rot,
		slot_key->batch_tex_unit,
		0.0f,
		0.0f,
		opacity,

		(1.0f - origin.x) * scale.x,
		(0.0f - origin.y) * scale.y,
		pos.x,
		pos.y,
		user_tex_size.x,
		user_tex_size.y,
		rot,
		slot_key->batch_tex_unit,
		1.0f,
		0.0f,
		opacity,

		(1.0f - origin.x) * scale.x,
		(1.0f - origin.y) * scale.y,
		pos.x,
		pos.y,
		user_tex_size.x,
		user_tex_size.y,
		rot,
		slot_key->batch_tex_unit,
		1.0f,
		1.0f,
		opacity,

		(0.0f - origin.x) * scale.x,
		(1.0f - origin.y) * scale.y,
		pos.x,
		pos.y,
		user_tex_size.x,
		user_tex_size.y,
		rot,
		slot_key->batch_tex_unit,
		0.0f,
		1.0f,
		opacity
	};

	glBindVertexArray(batch_data->vert_array_gl_ids[layer_index][slot_key->batch_index]);
	glBindBuffer(GL_ARRAY_BUFFER, batch_data->vert_buf_gl_ids[layer_index][slot_key->batch_index]); // NOTE: This might be redundant.
	glBufferSubData(GL_ARRAY_BUFFER, slot_key->batch_slot_index * sizeof(verts), sizeof(verts), verts);
}

zfw_vec_2d_t zfw_get_view_to_screen_pos(const zfw_vec_2d_t pos, const zfw_view_state_t *const view_state)
{
	return zfw_get_vec_2d_scaled(zfw_create_vec_2d(pos.x - view_state->pos.x, pos.y - view_state->pos.y), view_state->scale);
}

zfw_vec_2d_t zfw_get_screen_to_view_pos(const zfw_vec_2d_t pos, const zfw_view_state_t *const view_state)
{
	return zfw_get_vec_2d_sum(view_state->pos, zfw_get_vec_2d_scaled(pos, 1.0f / view_state->scale));
}

static void clean_game(zfw_game_cleanup_info_t *const cleanup_info)
{
	zfw_log("Cleaning up...");

	// Clean sprite batch data.
	clean_sprite_batch_data(cleanup_info->screen_sprite_batch_data);
	clean_sprite_batch_data(cleanup_info->view_sprite_batch_data);

	// Clean built-in shader program data.
	if (cleanup_info->builtin_shader_prog_data)
	{
		glDeleteProgram(cleanup_info->builtin_shader_prog_data->textured_rect_prog_gl_id);
	}

	// Clean user asset data.
	if (cleanup_info->user_asset_data)
	{
		for (int i = 0; i < cleanup_info->user_asset_data->shader_prog_data.prog_count; i++)
		{
			glDeleteProgram(cleanup_info->user_asset_data->shader_prog_data.gl_ids[i]);
		}

		free(cleanup_info->user_asset_data->shader_prog_data.gl_ids);

		free(cleanup_info->user_asset_data->tex_data.sizes);

		if (cleanup_info->user_asset_data->tex_data.gl_ids)
		{
			glDeleteTextures(cleanup_info->user_asset_data->tex_data.tex_count, cleanup_info->user_asset_data->tex_data.gl_ids);
			free(cleanup_info->user_asset_data->tex_data.gl_ids);
		}
	}

	// Uninitialize GLFW.
	glfwDestroyWindow(cleanup_info->glfw_window);
	glfwTerminate();
}

static void glfw_window_size_callback(GLFWwindow *glfw_window, int width, int height)
{
	zfw_glfw_window_user_data_t *const user_data = glfwGetWindowUserPointer(glfw_window);
	user_data->window_state.size = zfw_create_vec_2d_i(width, height);

	glViewport(0, 0, width, height);
}

static void glfw_window_pos_callback(GLFWwindow *glfw_window, int x, int y)
{
	zfw_glfw_window_user_data_t *const user_data = glfwGetWindowUserPointer(glfw_window);
	user_data->window_state.pos = zfw_create_vec_2d_i(x, y);
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
		case GLFW_KEY_SPACE: key_code = ZFW_KEY_CODE_SPACE; break;

		case GLFW_KEY_0: key_code = ZFW_KEY_CODE_0; break;
		case GLFW_KEY_1: key_code = ZFW_KEY_CODE_1; break;
		case GLFW_KEY_2: key_code = ZFW_KEY_CODE_2; break;
		case GLFW_KEY_3: key_code = ZFW_KEY_CODE_3; break;
		case GLFW_KEY_4: key_code = ZFW_KEY_CODE_4; break;
		case GLFW_KEY_5: key_code = ZFW_KEY_CODE_5; break;
		case GLFW_KEY_6: key_code = ZFW_KEY_CODE_6; break;
		case GLFW_KEY_7: key_code = ZFW_KEY_CODE_7; break;
		case GLFW_KEY_8: key_code = ZFW_KEY_CODE_8; break;
		case GLFW_KEY_9: key_code = ZFW_KEY_CODE_9; break;

		case GLFW_KEY_A: key_code = ZFW_KEY_CODE_A; break;
		case GLFW_KEY_B: key_code = ZFW_KEY_CODE_B; break;
		case GLFW_KEY_C: key_code = ZFW_KEY_CODE_C; break;
		case GLFW_KEY_D: key_code = ZFW_KEY_CODE_D; break;
		case GLFW_KEY_E: key_code = ZFW_KEY_CODE_E; break;
		case GLFW_KEY_F: key_code = ZFW_KEY_CODE_F; break;
		case GLFW_KEY_G: key_code = ZFW_KEY_CODE_G; break;
		case GLFW_KEY_H: key_code = ZFW_KEY_CODE_H; break;
		case GLFW_KEY_I: key_code = ZFW_KEY_CODE_I; break;
		case GLFW_KEY_J: key_code = ZFW_KEY_CODE_J; break;
		case GLFW_KEY_K: key_code = ZFW_KEY_CODE_K; break;
		case GLFW_KEY_L: key_code = ZFW_KEY_CODE_L; break;
		case GLFW_KEY_M: key_code = ZFW_KEY_CODE_M; break;
		case GLFW_KEY_N: key_code = ZFW_KEY_CODE_N; break;
		case GLFW_KEY_O: key_code = ZFW_KEY_CODE_O; break;
		case GLFW_KEY_P: key_code = ZFW_KEY_CODE_P; break;
		case GLFW_KEY_Q: key_code = ZFW_KEY_CODE_Q; break;
		case GLFW_KEY_R: key_code = ZFW_KEY_CODE_R; break;
		case GLFW_KEY_S: key_code = ZFW_KEY_CODE_S; break;
		case GLFW_KEY_T: key_code = ZFW_KEY_CODE_T; break;
		case GLFW_KEY_U: key_code = ZFW_KEY_CODE_U; break;
		case GLFW_KEY_V: key_code = ZFW_KEY_CODE_V; break;
		case GLFW_KEY_W: key_code = ZFW_KEY_CODE_W; break;
		case GLFW_KEY_X: key_code = ZFW_KEY_CODE_X; break;
		case GLFW_KEY_Y: key_code = ZFW_KEY_CODE_Y; break;
		case GLFW_KEY_Z: key_code = ZFW_KEY_CODE_Z; break;

		case GLFW_KEY_ESCAPE: key_code = ZFW_KEY_CODE_ESCAPE; break;
		case GLFW_KEY_ENTER: key_code = ZFW_KEY_CODE_ENTER; break;
		case GLFW_KEY_TAB: key_code = ZFW_KEY_CODE_TAB; break;

		case GLFW_KEY_RIGHT: key_code = ZFW_KEY_CODE_RIGHT; break;
		case GLFW_KEY_LEFT: key_code = ZFW_KEY_CODE_LEFT; break;
		case GLFW_KEY_DOWN: key_code = ZFW_KEY_CODE_DOWN; break;
		case GLFW_KEY_UP: key_code = ZFW_KEY_CODE_UP; break;

		case GLFW_KEY_F1: key_code = ZFW_KEY_CODE_F1; break;
		case GLFW_KEY_F2: key_code = ZFW_KEY_CODE_F2; break;
		case GLFW_KEY_F3: key_code = ZFW_KEY_CODE_F3; break;
		case GLFW_KEY_F4: key_code = ZFW_KEY_CODE_F4; break;
		case GLFW_KEY_F5: key_code = ZFW_KEY_CODE_F5; break;
		case GLFW_KEY_F6: key_code = ZFW_KEY_CODE_F6; break;
		case GLFW_KEY_F7: key_code = ZFW_KEY_CODE_F7; break;
		case GLFW_KEY_F8: key_code = ZFW_KEY_CODE_F8; break;
		case GLFW_KEY_F9: key_code = ZFW_KEY_CODE_F9; break;
		case GLFW_KEY_F10: key_code = ZFW_KEY_CODE_F10; break;
		case GLFW_KEY_F11: key_code = ZFW_KEY_CODE_F11; break;
		case GLFW_KEY_F12: key_code = ZFW_KEY_CODE_F12; break;

		case GLFW_KEY_LEFT_SHIFT: key_code = ZFW_KEY_CODE_LEFT_SHIFT; break;
		case GLFW_KEY_LEFT_CONTROL: key_code = ZFW_KEY_CODE_LEFT_CONTROL; break;
		case GLFW_KEY_LEFT_ALT: key_code = ZFW_KEY_CODE_LEFT_ALT; break;
	}

	if (key_code != -1)
	{
		zfw_glfw_window_user_data_t *const user_data = glfwGetWindowUserPointer(glfw_window);
		const zfw_keys_down_bits_t bitmask = (zfw_keys_down_bits_t)1 << key_code;

		if (glfw_action == GLFW_PRESS)
		{
			user_data->input_state.keys_down_bits |= bitmask;
		}
		else
		{
			user_data->input_state.keys_down_bits &= ~bitmask;
		}
	}
}

static void glfw_mouse_button_callback(GLFWwindow *glfw_window, int glfw_button_index, int glfw_action, int glfw_mods)
{
	zfw_glfw_window_user_data_t *const user_data = glfwGetWindowUserPointer(glfw_window);
	const zfw_mouse_buttons_down_bits_t bitmask = (zfw_mouse_buttons_down_bits_t)1 << glfw_button_index;

	if (glfw_action == GLFW_PRESS)
	{
		user_data->input_state.mouse_buttons_down_bits |= bitmask;
	}
	else
	{
		user_data->input_state.mouse_buttons_down_bits &= ~bitmask;
	}
}

static void glfw_mouse_pos_callback(GLFWwindow *glfw_window, double mouse_pos_x, double mouse_pos_y)
{
	zfw_glfw_window_user_data_t *const user_data = glfwGetWindowUserPointer(glfw_window);
	user_data->input_state.mouse_pos.x = (float)mouse_pos_x;
	user_data->input_state.mouse_pos.y = (float)mouse_pos_y;
}

static void glfw_joystick_callback(int glfw_joystick_index, int glfw_event)
{
	zfw_glfw_window_user_data_t *const user_data = glfwGetWindowUserPointer(glfwGetCurrentContext());

	if (glfw_event == GLFW_DISCONNECTED && user_data->input_state.gamepad_glfw_joystick_index == glfw_joystick_index)
	{
		reset_gamepad_state(&user_data->input_state);
		zfw_log("Gamepad with GLFW joystick index %d was disconnected.", glfw_joystick_index);
	}
	else if (glfw_event == GLFW_CONNECTED && user_data->input_state.gamepad_glfw_joystick_index == -1 && glfwJoystickIsGamepad(glfw_joystick_index))
	{
		reset_gamepad_state(&user_data->input_state);
		user_data->input_state.gamepad_glfw_joystick_index = glfw_joystick_index;

		zfw_log("Connected gamepad with GLFW joystick index %d.", glfw_joystick_index);
	}
}

static void reset_gamepad_state(zfw_input_state_t *const input_state)
{
	input_state->gamepad_glfw_joystick_index = -1;

	input_state->gamepad_buttons_down_bits = 0;
	memset(&input_state->gamepad_axis_values, 0, sizeof(input_state->gamepad_axis_values));

	zfw_log("Gamepad state reset.");
}

static void init_sprite_batch_data(zfw_sprite_batch_data_t *const batch_data)
{
	memset(batch_data, 0, sizeof(*batch_data));

	glGenVertexArrays(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_data->vert_array_gl_ids);
	glGenBuffers(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_data->vert_buf_gl_ids);
	glGenBuffers(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_data->elem_buf_gl_ids);

	memset(batch_data->user_tex_indexes, -1, sizeof(batch_data->user_tex_indexes));
}

static zfw_bool_t init_and_activate_render_layer_sprite_batch(const int layer_index, const int batch_index, zfw_sprite_batch_data_t *const batch_data)
{
	glBindVertexArray(batch_data->vert_array_gl_ids[layer_index][batch_index]);

	glBindBuffer(GL_ARRAY_BUFFER, batch_data->vert_buf_gl_ids[layer_index][batch_index]);

	{
		float *verts_buf;
		const int verts_buf_size = sizeof(*verts_buf) * 44 * ZFW_SPRITE_BATCH_SLOT_LIMIT;
		verts_buf = malloc(verts_buf_size);

		if (!verts_buf)
		{
			zfw_log_error("Failed to allocate %d bytes for render layer sprite batch vertices!", verts_buf_size);
			return ZFW_FALSE;
		}

		memset(verts_buf, 0, verts_buf_size);

		glBufferData(GL_ARRAY_BUFFER, verts_buf_size, verts_buf, GL_DYNAMIC_DRAW);

		free(verts_buf);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch_data->elem_buf_gl_ids[layer_index][batch_index]);

	{
		unsigned short *indices_buf;
		const int indices_buf_size = sizeof(*indices_buf) * 6 * ZFW_SPRITE_BATCH_SLOT_LIMIT;
		indices_buf = malloc(indices_buf_size);

		if (!indices_buf)
		{
			zfw_log_error("Failed to allocate %d bytes for render layer sprite batch elements!", indices_buf_size);
			return ZFW_FALSE;
		}

		for (int i = 0; i < ZFW_SPRITE_BATCH_SLOT_LIMIT; i++)
		{
			indices_buf[(i * 6) + 0] = (i * 4) + 0;
			indices_buf[(i * 6) + 1] = (i * 4) + 1;
			indices_buf[(i * 6) + 2] = (i * 4) + 2;
			indices_buf[(i * 6) + 3] = (i * 4) + 2;
			indices_buf[(i * 6) + 4] = (i * 4) + 3;
			indices_buf[(i * 6) + 5] = (i * 4) + 0;
		}

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_buf_size, indices_buf, GL_STATIC_DRAW);

		free(indices_buf);
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

	batch_data->batch_activities[layer_index] |= (zfw_render_layer_sprite_batch_activity_bits_t)1 << batch_index;

	return ZFW_TRUE;
}

void draw_sprite_batches(zfw_sprite_batch_data_t *const batch_data, const zfw_user_tex_data_t *const user_tex_data, const zfw_builtin_shader_prog_data_t *const builtin_shader_prog_data)
{
	for (int i = 0; i < ZFW_RENDER_LAYER_LIMIT; i++)
	{
		if (!batch_data->batch_activities[i])
		{
			// All the batches of this layer are inactive, so continue to the next one.
			continue;
		}

		for (int j = 0; j < ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT; j++)
		{
			if (!(batch_data->batch_activities[i] & ((zfw_render_layer_sprite_batch_activity_bits_t)1 << j)))
			{
				continue;
			}

			int tex_units[ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT] = { 0 };

			for (int k = 0; k < ZFW_SPRITE_BATCH_TEX_UNIT_LIMIT; k++)
			{
				const int user_tex_index = batch_data->user_tex_indexes[i][j][k];

				if (user_tex_index != -1)
				{
					tex_units[k] = k;

					glActiveTexture(GL_TEXTURE0 + k);
					glBindTexture(GL_TEXTURE_2D, user_tex_data->gl_ids[user_tex_index]);
				}
			}

			glUniform1iv(glGetUniformLocation(builtin_shader_prog_data->textured_rect_prog_gl_id, "u_textures"), ZFW_ARRAY_LEN(tex_units), tex_units);

			glBindVertexArray(batch_data->vert_array_gl_ids[i][j]);
			glDrawElements(GL_TRIANGLES, 6 * ZFW_SPRITE_BATCH_SLOT_LIMIT, GL_UNSIGNED_SHORT, NULL);
		}
	}
}

static void clean_sprite_batch_data(zfw_sprite_batch_data_t *const batch_data)
{
	glDeleteBuffers(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_data->elem_buf_gl_ids);
	glDeleteBuffers(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_data->vert_buf_gl_ids);
	glDeleteVertexArrays(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_data->vert_array_gl_ids);
}
