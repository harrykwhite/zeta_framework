#include <zfw_game.h>

#include <time.h>
#include <GLFW/glfw3.h>
#include <zfw_common_debug.h>

#define TARG_TICKS_PER_SEC 60
#define TARG_TICK_INTERVAL (1.0 / TARG_TICKS_PER_SEC)

typedef struct
{
    zfw_mem_arena_t *main_mem_arena;

    GLFWwindow *glfw_window;

    zfw_user_tex_data_t *user_tex_data;
    zfw_user_shader_prog_data_t *user_shader_prog_data;
    zfw_user_font_data_t *user_font_data;
    zfw_builtin_shader_prog_data_t *builtin_shader_prog_data;

    zfw_sprite_batch_group_t *sprite_batch_groups;
    int sprite_batch_groups_cleanup_count;

    zfw_char_batch_group_t *char_batch_group;
} game_cleanup_data_t;

typedef struct
{
    zfw_vec_2d_i_t size;
    zfw_vec_2d_i_t pos;
    zfw_bool_t fullscreen;
} window_state_t;

typedef struct
{
    window_state_t *const window_state;
    zfw_input_state_t *const input_state;
} glfw_window_callback_data_t;

const zfw_color_t k_default_bg_color = {0.2f, 0.075f, 0.15f, 1.0f};

static void clean_game(game_cleanup_data_t *const cleanup_data)
{
    zfw_log("Cleaning up...");

    // Clean sprite and character batch groups.
    if (cleanup_data->char_batch_group)
    {
        zfw_clean_char_batch_group(cleanup_data->char_batch_group);
    }

    for (int i = 0; i < cleanup_data->sprite_batch_groups_cleanup_count; i++)
    {
        zfw_clean_sprite_batch_group(&cleanup_data->sprite_batch_groups[i]);
    }

    // Clean built-in shader program data.
    if (cleanup_data->builtin_shader_prog_data)
    {
        glDeleteProgram(cleanup_data->builtin_shader_prog_data->char_quad_prog_gl_id);
        glDeleteProgram(cleanup_data->builtin_shader_prog_data->sprite_quad_prog_gl_id);
    }

    // Clean user asset data.
    if (cleanup_data->user_font_data && cleanup_data->user_font_data->font_count && cleanup_data->user_font_data->tex_gl_ids)
    {
        glDeleteTextures(cleanup_data->user_font_data->font_count, cleanup_data->user_font_data->tex_gl_ids);
    }

    if (cleanup_data->user_shader_prog_data && cleanup_data->user_shader_prog_data->gl_ids)
    {
        for (int i = 0; i < cleanup_data->user_shader_prog_data->prog_count; i++)
        {
            glDeleteProgram(cleanup_data->user_shader_prog_data->gl_ids[i]);
        }
    }

    if (cleanup_data->user_tex_data && cleanup_data->user_tex_data->tex_count && cleanup_data->user_tex_data->gl_ids)
    {
        glDeleteTextures(cleanup_data->user_tex_data->tex_count, cleanup_data->user_tex_data->gl_ids);
    }

    // Uninitialise GLFW.
    if (cleanup_data->glfw_window)
    {
        glfwDestroyWindow(cleanup_data->glfw_window);
    }

    glfwTerminate();

    // Clean memory arenas.
    if (cleanup_data->main_mem_arena)
    {
        zfw_clean_mem_arena(cleanup_data->main_mem_arena);
    }
}

static double calc_frame_time_change(const double frame_time, const double frame_time_last)
{
    double change = frame_time - frame_time_last;

    // Handle frame time anomalies.
    if (change > TARG_TICK_INTERVAL * 8)
    {
        change = TARG_TICK_INTERVAL;
    }

    if (change < 0.0)
    {
        change = 0.0;
    }

    return change;
}

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

static void glfw_key_callback(GLFWwindow *glfw_window, int glfw_key_code, int glfw_scancode, int glfw_action, int glfw_mods)
{
    if (glfw_action == GLFW_REPEAT)
    {
        return;
    }

    const zfw_key_code_t key_code = zfw_get_glfw_to_zfw_key_code(glfw_key_code);

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

zfw_bool_t zfw_run_game(const zfw_user_game_run_info_t *const user_run_info)
{
    //
    // Initialisation
    //

    // Ensure data type sizes meet requirements before proceeding.
    if (!zfw_check_data_type_sizes())
    {
        return ZFW_FALSE;
    }

    zfw_log("Data type sizes meet requirements.");

    // Initialise the random number generator.
    srand(time(NULL));
    zfw_log("Initialized the random number generator.");

    // Create and zero-out the game cleanup data struct.
    game_cleanup_data_t cleanup_data = {0};

    // Initialise the memory arena.
    zfw_mem_arena_t main_mem_arena;

    if (!zfw_init_mem_arena(&main_mem_arena, ZFW_MAIN_MEM_ARENA_SIZE))
    {
        zfw_log_error("Failed to initialize the main memory arena! (Size: %d bytes)", ZFW_MAIN_MEM_ARENA_SIZE);
        clean_game(&cleanup_data);
        return ZFW_FALSE;
    }

    zfw_log("Successfully initialized the main memory arena! (Size: %d bytes)", ZFW_MAIN_MEM_ARENA_SIZE);

    cleanup_data.main_mem_arena = &main_mem_arena;

    // Initialise GLFW.
    if (!glfwInit())
    {
        zfw_log_error("Failed to initialize GLFW!");
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
        clean_game(&cleanup_data);
        return ZFW_FALSE;
    }

    zfw_log("Successfully created a GLFW window!");

    cleanup_data.glfw_window = glfw_window;

    glfwMakeContextCurrent(glfw_window);

    // Initialise the window state.
    window_state_t window_state = {0};
    glfwGetWindowSize(glfw_window, &window_state.size.x, &window_state.size.y);
    glfwGetWindowPos(glfw_window, &window_state.pos.x, &window_state.pos.y);

    // Initialise the input state.
    zfw_input_state_t input_state = {0};

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
    glfw_window_callback_data_t glfw_window_callback_data = {&window_state, &input_state};
    glfwSetWindowUserPointer(glfw_window, &glfw_window_callback_data);

    glfwSetWindowSizeCallback(glfw_window, glfw_window_size_callback);
    glfwSetWindowPosCallback(glfw_window, glfw_window_pos_callback);

    glfwSetKeyCallback(glfw_window, glfw_key_callback);
    glfwSetMouseButtonCallback(glfw_window, glfw_mouse_button_callback);
    glfwSetScrollCallback(glfw_window, glfw_mouse_scroll_callback);
    glfwSetCursorPosCallback(glfw_window, glfw_mouse_pos_callback);
    glfwSetJoystickCallback(glfw_joystick_callback);

    // Set cursor visibility.
    glfwSetInputMode(glfw_window, GLFW_CURSOR, user_run_info->hide_cursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

    // Set up OpenGL function pointers.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        zfw_log_error("Failed to initialize OpenGL function pointers!");
        clean_game(&cleanup_data);
        return ZFW_FALSE;
    }

    zfw_log("Successfully set up OpenGL function pointers!");

    // Initialise user asset data.
    zfw_user_tex_data_t user_tex_data;
    zfw_user_shader_prog_data_t user_shader_prog_data;
    zfw_user_font_data_t user_font_data;

    {
        FILE *const assets_file_fs = fopen(ZFW_ASSETS_FILE_NAME, "rb");

        if (!assets_file_fs)
        {
            zfw_log_error("Failed to open assets file \"%s\"!", ZFW_ASSETS_FILE_NAME);
            clean_game(&cleanup_data);
            return ZFW_FALSE;
        }

        cleanup_data.user_tex_data = &user_tex_data;
        cleanup_data.user_shader_prog_data = &user_shader_prog_data;
        cleanup_data.user_font_data = &user_font_data;

        zfw_log("Retrieving user asset data from \"%s\"...", ZFW_ASSETS_FILE_NAME);
        const zfw_bool_t asset_data_read_successful = zfw_retrieve_user_asset_data_from_assets_file(&user_tex_data, &user_shader_prog_data, &user_font_data, assets_file_fs, &main_mem_arena);

        fclose(assets_file_fs);

        if (!asset_data_read_successful)
        {
            clean_game(&cleanup_data);
            return ZFW_FALSE;
        }
    }

    // Initialise built-in shader programs.
    zfw_builtin_shader_prog_data_t builtin_shader_prog_data = {0};

    zfw_gen_shader_prog(&builtin_shader_prog_data.sprite_quad_prog_gl_id, ZFW_BUILTIN_SPRITE_QUAD_VERT_SHADER_SRC, ZFW_BUILTIN_SPRITE_QUAD_FRAG_SHADER_SRC);
    zfw_gen_shader_prog(&builtin_shader_prog_data.char_quad_prog_gl_id, ZFW_BUILTIN_CHAR_QUAD_VERT_SHADER_SRC, ZFW_BUILTIN_CHAR_QUAD_FRAG_SHADER_SRC);

    zfw_log("Initialized built-in shader programs!");

    cleanup_data.builtin_shader_prog_data = &builtin_shader_prog_data;

    // Set up rendering.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    zfw_sprite_batch_group_t sprite_batch_groups[ZFW_SPRITE_BATCH_GROUP_COUNT] = {0};
    cleanup_data.sprite_batch_groups = sprite_batch_groups;

    for (int i = 0; i < ZFW_SPRITE_BATCH_GROUP_COUNT; i++)
    {
        if (!zfw_init_sprite_batch_group(&sprite_batch_groups[i], &main_mem_arena))
        {
            clean_game(&cleanup_data);
            return ZFW_FALSE;
        }

        cleanup_data.sprite_batch_groups_cleanup_count++;
    }

    zfw_log("Successfully set up sprite batch groups!");

    zfw_char_batch_group_t char_batch_group = {0};

    if (!zfw_init_char_batch_group(&char_batch_group, &main_mem_arena))
    {
        clean_game(&cleanup_data);
        return ZFW_FALSE;
    }

    cleanup_data.char_batch_group = &char_batch_group;

    zfw_log("Successfully set up the character batch group!");

    zfw_view_state_t view_state;

    // Show the window now that things have been set up.
    zfw_log("Showing the GLFW window...");
    glfwShowWindow(glfw_window);

    // This represents whether or not the user wants the window to be in fullscreen, assignable by them through pointers passed into their game functions. The actual state will not be updated until a specific point in the main loop.
    zfw_bool_t user_window_fullscreen = user_run_info->init_window_fullscreen;

    // This is to be a copy of the input state at the point of the last tick.
    zfw_input_state_t last_tick_input_state = {0};

    // This is to be a copy of the window state prior to switching from windowed mode to fullscreen, so that this state can be returned to when switching back.
    window_state_t window_prefullscreen_state;

    // Set rendering defaults.
    for (int i = 0; i < ZFW_SPRITE_BATCH_GROUP_COUNT; i++)
    {
        zfw_set_sprite_batch_group_defaults(&sprite_batch_groups[i]);
    }

    zfw_set_char_batch_group_defaults(&char_batch_group);

    zfw_set_view_state_defaults(&view_state);

    // This is the data provided to the user in their defined game functions.
    zfw_user_func_data_t user_func_data;
    user_func_data.main_mem_arena = &main_mem_arena;
    user_func_data.window_size = window_state.size;
    user_func_data.window_fullscreen = &user_window_fullscreen;
    user_func_data.input_state = &input_state;
    user_func_data.input_state_last = &last_tick_input_state;
    user_func_data.user_tex_data = &user_tex_data;
    user_func_data.user_shader_prog_data = &user_shader_prog_data;
    user_func_data.user_font_data = &user_font_data;
    user_func_data.sprite_batch_groups = sprite_batch_groups;
    user_func_data.char_batch_group = &char_batch_group;
    user_func_data.view_state = &view_state;

    // Run the user-defined game initialisation function.
    user_run_info->on_init_func(user_run_info->user_ptr, &user_func_data);

    //
    // Main Loop
    //
    double frame_time = glfwGetTime();
    double frame_time_change_accum = TARG_TICK_INTERVAL; // The assignment here ensures that a tick is always run on the first frame.

    zfw_log("Entering the main loop...");

    while (!glfwWindowShouldClose(glfw_window))
    {
        const zfw_vec_2d_i_t window_size_last = window_state.size;

        glfwPollEvents();

        if (user_run_info->on_window_resize_func && (window_state.size.x != window_size_last.x || window_state.size.y != window_size_last.y))
        {
            // The user has defined a window resize function and the window has been resized, so call the function.
            user_func_data.window_size = window_state.size;
            user_run_info->on_window_resize_func(user_run_info->user_ptr, &user_func_data);
        }

        zfw_update_gamepad_state(&input_state);

        // Update frame time data.
        const double frame_time_last = frame_time;
        frame_time = glfwGetTime();
        const double frame_time_change = calc_frame_time_change(frame_time, frame_time_last);
        frame_time_change_accum += frame_time_change;

        // Calculate tick count and handle the case where at least one tick occurred.
        const int tick_count = (int)(frame_time_change_accum / TARG_TICK_INTERVAL);

        if (tick_count > 0)
        {
            // Run the ticks.
            for (int i = 0; i < tick_count; i++)
            {
                user_run_info->on_tick_func(user_run_info->user_ptr, &user_func_data, tick_count, frame_time_change_accum);
                frame_time_change_accum -= TARG_TICK_INTERVAL;
            }

            // Reset the mouse scroll now that the ticks are complete.
            input_state.mouse_scroll = 0;

            // Update the tick input state for next time.
            last_tick_input_state = input_state;

            // If the user has requested a change to the fullscreen state, update it.
            if (window_state.fullscreen != user_window_fullscreen)
            {
                zfw_bool_t call_user_window_resize_func = ZFW_TRUE;

                if (user_window_fullscreen)
                {
                    GLFWmonitor *const glfw_primary_monitor = glfwGetPrimaryMonitor();

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
                    user_func_data.window_size = window_state.size;
                    user_run_info->on_window_resize_func(user_run_info->user_ptr, &user_func_data);
                }
            }

            // Render.
            glClearColor(k_default_bg_color.r, k_default_bg_color.g, k_default_bg_color.b, k_default_bg_color.a);
            glClear(GL_COLOR_BUFFER_BIT);

            zfw_render_sprite_and_character_batches(sprite_batch_groups, &char_batch_group, &view_state, window_state.size, &user_tex_data, &user_font_data, &builtin_shader_prog_data);

            glfwSwapBuffers(glfw_window);
        }
    }

    clean_game(&cleanup_data);

    return ZFW_TRUE;
}
