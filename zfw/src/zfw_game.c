#include <zfw.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GLFW/glfw3.h>

////// Internal Structs //////
// Pointers to data to be cleaned up on game end. If a pointer is null, it should be because the data hasn't been set up yet and thus does not need to be cleaned.
typedef struct
{
    zfw_mem_arena_t *mem_arena;

    GLFWwindow *glfw_window;

    zfw_user_asset_data_t *user_asset_data;
    zfw_builtin_shader_prog_data_t *builtin_shader_prog_data;

    zfw_sprite_batch_group_t *sprite_batch_groups;
    int sprite_batch_groups_cleanup_count;

    zfw_char_batch_group_t *char_batch_group;
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

////// Static Rendering Functions //////
static zfw_bool_t init_sprite_batch_group(zfw_sprite_batch_group_t *const batch_group)
{
    glGenVertexArrays(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_group->vert_array_gl_ids);
    glGenBuffers(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_group->vert_buf_gl_ids);
    glGenBuffers(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_group->elem_buf_gl_ids);

    if (!zfw_init_bitset(&batch_group->slot_activity_bitset, (ZFW_SPRITE_BATCH_SLOT_LIMIT * ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT) / 8))
    {
        zfw_log_error("Failed to initialize a sprite batch slot activity bitset!");
        return ZFW_FALSE;
    }

    return ZFW_TRUE;
}

static void set_defaults_of_sprite_batch_groups(zfw_sprite_batch_group_t *const batch_groups)
{
    for (int i = 0; i < ZFW_SPRITE_BATCH_GROUP_ID_COUNT; i++)
    {
        memset(batch_groups[i].batch_activity_bits, 0, sizeof(batch_groups[i].batch_activity_bits));
        memset(batch_groups[i].tex_units, 0, sizeof(batch_groups[i].tex_units));
        zfw_clear_bitset(&batch_groups[i].slot_activity_bitset);
    }
}

static void draw_sprite_batches_of_layer(const int layer_index, zfw_sprite_batch_group_t *const batch_group, const zfw_user_tex_data_t *const user_tex_data, const zfw_builtin_shader_prog_data_t *const builtin_shader_prog_data)
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
            const int user_tex_index = batch_group->tex_units[layer_index][i][j].user_tex_index;

            if (user_tex_index != -1)
            {
                tex_units[j] = j;

                glActiveTexture(GL_TEXTURE0 + j);
                glBindTexture(GL_TEXTURE_2D, user_tex_data->gl_ids[user_tex_index]);
            }
        }

        glUniform1iv(glGetUniformLocation(builtin_shader_prog_data->textured_rect_prog_gl_id, "u_textures"), ZFW_STATIC_ARRAY_LEN(tex_units), tex_units);

        glBindVertexArray(batch_group->vert_array_gl_ids[layer_index][i]);
        glDrawElements(GL_TRIANGLES, 6 * ZFW_SPRITE_BATCH_SLOT_LIMIT, GL_UNSIGNED_SHORT, 0);
    }
}

static void clean_sprite_batch_group(zfw_sprite_batch_group_t *const batch_group)
{
    zfw_clean_bitset(&batch_group->slot_activity_bitset);

    glDeleteBuffers(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_group->elem_buf_gl_ids);
    glDeleteBuffers(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_group->vert_buf_gl_ids);
    glDeleteVertexArrays(ZFW_RENDER_LAYER_SPRITE_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_group->vert_array_gl_ids);
}

static void init_char_batch_group(zfw_char_batch_group_t *const batch_group)
{
    glGenVertexArrays(ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_group->vert_array_gl_ids);
    glGenBuffers(ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_group->vert_buf_gl_ids);
    glGenBuffers(ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_group->elem_buf_gl_ids);
}

static void set_char_batch_group_defaults(zfw_char_batch_group_t *const batch_group)
{
    memset(batch_group->batch_init_bits, 0, sizeof(batch_group->batch_init_bits));
    memset(batch_group->batch_activity_bits, 0, sizeof(batch_group->batch_activity_bits));
    memset(batch_group->user_font_indexes, 0, sizeof(batch_group->user_font_indexes));
    memset(batch_group->positions, 0, sizeof(batch_group->positions));

    for (int i = 0; i < ZFW_RENDER_LAYER_LIMIT; i++)
    {
        for (int j = 0; j < ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT; j++)
        {
            batch_group->scales[i][j].x = 1.0f;
            batch_group->scales[i][j].y = 1.0f;

            batch_group->blends[i][j].r = 1.0f;
            batch_group->blends[i][j].g = 1.0f;
            batch_group->blends[i][j].b = 1.0f;
            batch_group->blends[i][j].a = 1.0f;
        }
    }
}

static void draw_char_batches_of_layer(const int layer_index, zfw_char_batch_group_t *const batch_group, const zfw_user_font_data_t *const user_font_data, const zfw_builtin_shader_prog_data_t *const builtin_shader_prog_data)
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

        glUniform2fv(glGetUniformLocation(builtin_shader_prog_data->char_rect_prog_gl_id, "u_pos"), 1, (float *)&batch_group->positions[layer_index][i]);
        glUniform2fv(glGetUniformLocation(builtin_shader_prog_data->char_rect_prog_gl_id, "u_scale"), 1, (float *)&batch_group->scales[layer_index][i]);
        glUniform4fv(glGetUniformLocation(builtin_shader_prog_data->char_rect_prog_gl_id, "u_blend"), 1, (float *)&batch_group->blends[layer_index][i]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, user_font_data->tex_glids[batch_group->user_font_indexes[layer_index][i]]);

        glBindVertexArray(batch_group->vert_array_gl_ids[layer_index][i]);
        glDrawElements(GL_TRIANGLES, 6 * ZFW_CHAR_BATCH_SLOT_LIMIT, GL_UNSIGNED_SHORT, 0);
    }
}

static void clean_char_batch_group(zfw_char_batch_group_t *const batch_group)
{
    glDeleteBuffers(ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_group->elem_buf_gl_ids);
    glDeleteBuffers(ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_group->vert_buf_gl_ids);
    glDeleteVertexArrays(ZFW_RENDER_LAYER_CHAR_BATCH_LIMIT * ZFW_RENDER_LAYER_LIMIT, (GLuint *)batch_group->vert_array_gl_ids);
}

////// Static Game Functions //////
static void clean_game(game_cleanup_data_t *const cleanup_data)
{
    zfw_log("Cleaning up...");

    // Clean sprite and character batch data.
    if (cleanup_data->char_batch_group)
    {
        clean_char_batch_group(cleanup_data->char_batch_group);
    }

    for (int i = 0; i < cleanup_data->sprite_batch_groups_cleanup_count; i++)
    {
        clean_sprite_batch_group(&cleanup_data->sprite_batch_groups[i]);
    }

    // Clean built-in shader program data.
    if (cleanup_data->builtin_shader_prog_data)
    {
        glDeleteProgram(cleanup_data->builtin_shader_prog_data->char_rect_prog_gl_id);
        glDeleteProgram(cleanup_data->builtin_shader_prog_data->textured_rect_prog_gl_id);
    }

    // Clean user asset data.
    if (cleanup_data->user_asset_data)
    {
        if (cleanup_data->user_asset_data->font_data.tex_glids)
        {
            glDeleteTextures(cleanup_data->user_asset_data->font_data.font_count, cleanup_data->user_asset_data->font_data.tex_glids);
        }

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
    if (cleanup_data->mem_arena)
    {
        zfw_clean_mem_arena(cleanup_data->mem_arena);
    }
}
///////////////////////////////////

zfw_bool_t zfw_run_game(const zfw_user_game_run_info_t *const user_run_info)
{
    // Ensure data type sizes meet requirements before proceeding.
    if (!zfw_check_data_type_sizes())
    {
        return ZFW_FALSE;
    }

    zfw_log("Data type sizes meet requirements.");

    // Initialize the random number generator.
    srand(time(NULL));
    zfw_log("Initialized the random number generator.");

    // Create and zero-out the game cleanup data struct.
    game_cleanup_data_t cleanup_data = {0};

    // Initialize the main memory arena.
    zfw_mem_arena_t mem_arena;

    if (!zfw_init_mem_arena(&mem_arena, ZFW_MEM_ARENA_SIZE))
    {
        zfw_log_error("Failed to initialize the memory arena! (Size: %d bytes)", ZFW_MEM_ARENA_SIZE);
        clean_game(&cleanup_data);
        return ZFW_FALSE;
    }

    zfw_log("Successfully initialized the memory arena! (Size: %d bytes)", ZFW_MEM_ARENA_SIZE);

    // Initialize GLFW.
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

    // Initialize the window state.
    zfw_window_state_t window_state;
    glfwGetWindowSize(glfw_window, &window_state.size.x, &window_state.size.y);
    glfwGetWindowPos(glfw_window, &window_state.pos.x, &window_state.pos.y);
    window_state.fullscreen = ZFW_FALSE;

    // Initialize the input state.
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

    // Set up OpenGL function pointers.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        zfw_log_error("Failed to initialize OpenGL function pointers!");
        clean_game(&cleanup_data);
        return ZFW_FALSE;
    }

    zfw_log("Successfully set up OpenGL function pointers!");

    // Initialize user asset data.
    zfw_user_asset_data_t user_asset_data = {0};

    {
        FILE *const assets_file_fs = fopen(ZFW_ASSETS_FILE_NAME, "rb");

        if (!assets_file_fs)
        {
            zfw_log_error("Failed to open assets file \"%s\"!", ZFW_ASSETS_FILE_NAME);
            clean_game(&cleanup_data);
            return ZFW_FALSE;
        }

        zfw_log("Retrieving user asset data from \"%s\"...", ZFW_ASSETS_FILE_NAME);
        const zfw_bool_t asset_data_read_successful = zfw_retrieve_user_asset_data_from_assets_file(&user_asset_data, assets_file_fs, &mem_arena);

        fclose(assets_file_fs);

        if (!asset_data_read_successful)
        {
            clean_game(&cleanup_data);
            return ZFW_FALSE;
        }

        cleanup_data.user_asset_data = &user_asset_data;
    }

    // Initialize built-in shader programs.
    zfw_builtin_shader_prog_data_t builtin_shader_prog_data;

    zfw_gen_shader_prog(&builtin_shader_prog_data.textured_rect_prog_gl_id, ZFW_BUILTIN_TEXTURED_RECT_VERT_SHADER_SRC, ZFW_BUILTIN_TEXTURED_RECT_FRAG_SHADER_SRC);
    zfw_gen_shader_prog(&builtin_shader_prog_data.char_rect_prog_gl_id, ZFW_BUILTIN_CHAR_RECT_VERT_SHADER_SRC, ZFW_BUILTIN_CHAR_RECT_FRAG_SHADER_SRC);

    cleanup_data.builtin_shader_prog_data = &builtin_shader_prog_data;

    zfw_log("Successfully initialized built-in shader programs!");

    // Set up rendering.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    zfw_sprite_batch_group_t sprite_batch_groups[ZFW_SPRITE_BATCH_GROUP_ID_COUNT];
    cleanup_data.sprite_batch_groups = sprite_batch_groups;

    for (int i = 0; i < ZFW_SPRITE_BATCH_GROUP_ID_COUNT; i++)
    {
        if (!init_sprite_batch_group(&sprite_batch_groups[i]))
        {
            clean_game(&cleanup_data);
            return ZFW_FALSE;
        }

        cleanup_data.sprite_batch_groups_cleanup_count++;
    }

    zfw_log("Successfully set up sprite batch groups!");

    zfw_char_batch_group_t char_batch_group;
    init_char_batch_group(&char_batch_group);
    cleanup_data.char_batch_group = &char_batch_group;

    zfw_log("Set up the character batch group!");

    zfw_view_state_t view_state;

    // Show the window now that things have been set up.
    zfw_log("Showing the GLFW window...");
    glfwShowWindow(glfw_window);

    // Perform the restart loop.
    zfw_bool_t restart = ZFW_FALSE; // Represents whether or not to break out of the main loop and restart the game. This is modifiable by the user in their game functions through a pointer.

    zfw_bool_t user_window_fullscreen = user_run_info->init_window_fullscreen; // Represents whether or not the user wants the window to be in fullscreen, assignable by them through pointers passed into their game functions. The actual state will not be updated until a specific point in the main loop.

    zfw_input_state_t last_tick_input_state = {0}; // This is to be a copy of the input state at the point of the last tick.
    
    zfw_user_game_func_data_t user_func_data;
    user_func_data.mem_arena = &mem_arena;
    user_func_data.restart = &restart;
    user_func_data.window_size = window_state.size;
    user_func_data.window_fullscreen = &user_window_fullscreen;
    user_func_data.input_state = &input_state;
    user_func_data.input_state_last = &last_tick_input_state;
    user_func_data.user_asset_data = &user_asset_data;
    user_func_data.sprite_batch_groups = sprite_batch_groups;
    user_func_data.char_batch_group = &char_batch_group;
    user_func_data.view_state = &view_state;

    zfw_window_state_t window_prefullscreen_state; // This is to be a copy of the window state prior to switching from windowed mode to fullscreen, so that this state can be returned to when switching back.

    zfw_log("Entering the restart loop...");

    while (ZFW_TRUE)
    {
        zfw_log("Setting sprite group and character batch group defaults...");
        set_defaults_of_sprite_batch_groups(sprite_batch_groups);
        set_char_batch_group_defaults(&char_batch_group);

        zfw_reset_view_state(&view_state);

        // Run the user-defined game initialization function.
        user_run_info->on_init_func(user_run_info->user_ptr, &user_func_data);

        // Perform the main loop.
        double frame_time_last = glfwGetTime();
        const int target_ticks_per_sec = 60;
        const double target_tick_interval = 1.0 / target_ticks_per_sec;
        double frame_time_interval_accum = target_tick_interval; // (The assignment here ensures that a tick is always run on the first frame.)

        zfw_bool_t glfw_window_should_close;

        zfw_log("Entering the main loop...");

        while (!(glfw_window_should_close = glfwWindowShouldClose(glfw_window)) && !restart)
        {
            const zfw_vec_2d_i_t window_size_last = window_state.size;

            glfwPollEvents();

            // If the user has defined a window resize function and the window has been resized, call the function.
            if (user_run_info->on_window_resize_func && (window_state.size.x != window_size_last.x || window_state.size.y != window_size_last.y))
            {
                user_func_data.window_size = window_state.size;
                user_run_info->on_window_resize_func(user_run_info->user_ptr, &user_func_data);
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
            double frame_time_change = frame_time - frame_time_last;

            // Handle frame time anomalies.
            if (frame_time_change > target_tick_interval * 8.0)
            {
                frame_time_change = target_tick_interval;
            }

            if (frame_time_change < 0.0)
            {
                frame_time_change = 0.0;
            }
            //

            frame_time_interval_accum += frame_time_change;
            frame_time_last = frame_time;

            int tick_count = (int)(frame_time_interval_accum / target_tick_interval);

            if (tick_count > 0)
            {
                // Run the ticks.
                for (int i = 0; i < tick_count; i++)
                {
                    user_run_info->on_tick_func(user_run_info->user_ptr, &user_func_data);
                    frame_time_interval_accum -= target_tick_interval;
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
                        user_func_data.window_size = window_state.size;
                        user_run_info->on_window_resize_func(user_run_info->user_ptr, &user_func_data);
                    }
                }

                // Clear the screen.
                glClearColor(0.2f, 0.075f, 0.15f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                //

                zfw_matrix_4x4_t proj;
                zfw_init_ortho_matrix_4x4(&proj, 0.0f, window_state.size.x, window_state.size.y, 0.0f, -1.0f, 1.0f);

                // Draw view sprite batches.
                {
                    glUseProgram(builtin_shader_prog_data.textured_rect_prog_gl_id);

                    zfw_matrix_4x4_t view;
                    zfw_init_identity_matrix_4x4(&view);
                    view.elems[0][0] = view_state.scale;
                    view.elems[1][1] = view_state.scale;
                    view.elems[2][2] = 1.0f;
                    view.elems[3][0] = -view_state.pos.x * view_state.scale;
                    view.elems[3][1] = -view_state.pos.y * view_state.scale;
                    glUniformMatrix4fv(glGetUniformLocation(builtin_shader_prog_data.textured_rect_prog_gl_id, "u_view"), 1, GL_FALSE, (GLfloat *)view.elems);

                    glUniformMatrix4fv(glGetUniformLocation(builtin_shader_prog_data.textured_rect_prog_gl_id, "u_proj"), 1, GL_FALSE, (GLfloat *)proj.elems);

                    for (int i = 0; i < ZFW_RENDER_LAYER_LIMIT; i++)
                    {
                        draw_sprite_batches_of_layer(i, &sprite_batch_groups[ZFW_SPRITE_BATCH_GROUP_ID__VIEW], &user_asset_data.tex_data, &builtin_shader_prog_data);
                    }
                }

                // Draw screen sprite batches and character batches.
                for (int i = 0; i < ZFW_RENDER_LAYER_LIMIT; i++)
                {
                    // Draw layer screen sprite batches.
                    glUseProgram(builtin_shader_prog_data.textured_rect_prog_gl_id);

                    zfw_matrix_4x4_t view;
                    zfw_init_identity_matrix_4x4(&view);
                    glUniformMatrix4fv(glGetUniformLocation(builtin_shader_prog_data.textured_rect_prog_gl_id, "u_view"), 1, GL_FALSE, (GLfloat *)view.elems);

                    glUniformMatrix4fv(glGetUniformLocation(builtin_shader_prog_data.textured_rect_prog_gl_id, "u_proj"), 1, GL_FALSE, (GLfloat *)proj.elems);

                    draw_sprite_batches_of_layer(i, &sprite_batch_groups[ZFW_SPRITE_BATCH_GROUP_ID__SCREEN], &user_asset_data.tex_data, &builtin_shader_prog_data);

                    // Draw layer character batches.
                    glUseProgram(builtin_shader_prog_data.char_rect_prog_gl_id);
                    glUniformMatrix4fv(glGetUniformLocation(builtin_shader_prog_data.char_rect_prog_gl_id, "u_proj"), 1, GL_FALSE, (GLfloat *)proj.elems);
                    draw_char_batches_of_layer(i, &char_batch_group, &user_asset_data.font_data, &builtin_shader_prog_data);
                }
                //

                glfwSwapBuffers(glfw_window);
            }
        }
        //

        if (!restart || glfw_window_should_close)
        {
            break;
        }

        restart = ZFW_FALSE;
    };
    //

    clean_game(&cleanup_data);

    return ZFW_TRUE;
}
