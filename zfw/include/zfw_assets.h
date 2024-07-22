#ifndef __ZFW_ASSETS_H__
#define __ZFW_ASSETS_H__

#include <stdio.h>
#include <glad/glad.h>
#include <zfw_common.h>

#define ZFW_BUILTIN_SPRITE_QUAD_VERT_SHADER_SRC \
    "#version 430 core\n" \
    "\n" \
    "layout (location = 0) in vec2 a_vert;\n" \
    "layout (location = 1) in vec2 a_pos;\n" \
    "layout (location = 2) in vec2 a_size;\n" \
    "layout (location = 3) in float a_rot;\n" \
    "layout (location = 4) in float a_tex_index;\n" \
    "layout (location = 5) in vec2 a_tex_coord;\n" \
    "layout (location = 6) in vec4 a_blend;\n" \
    "\n" \
    "out flat int v_tex_index;\n" \
    "out vec2 v_tex_coord;\n" \
    "out vec4 v_blend;\n" \
    "\n" \
    "uniform mat4 u_view;\n" \
    "uniform mat4 u_proj;\n" \
    "\n" \
    "void main()\n" \
    "{\n" \
    "    float rot_cos = cos(a_rot);\n" \
    "    float rot_sin = -sin(a_rot);\n" \
    "\n" \
    "    mat4 model = mat4(\n" \
    "        vec4(a_size.x * rot_cos, a_size.x * rot_sin, 0.0f, 0.0f),\n" \
    "        vec4(a_size.y * -rot_sin, a_size.y * rot_cos, 0.0f, 0.0f),\n" \
    "        vec4(0.0f, 0.0f, 1.0f, 0.0f),\n" \
    "        vec4(a_pos.x, a_pos.y, 0.0f, 1.0f)\n" \
    "    );\n" \
    "\n" \
    "    gl_Position = u_proj * u_view * model * vec4(a_vert, 0.0f, 1.0f);\n" \
    "\n" \
    "    v_tex_index = int(a_tex_index);\n" \
    "    v_tex_coord = a_tex_coord;\n" \
    "    v_blend = a_blend;\n" \
    "}\n"

#define ZFW_BUILTIN_SPRITE_QUAD_FRAG_SHADER_SRC \
    "#version 430 core\n" \
    "\n" \
    "in flat int v_tex_index;\n" \
    "in vec2 v_tex_coord;\n" \
    "in vec4 v_blend;\n" \
    "\n" \
    "out vec4 o_frag_color;\n" \
    "\n" \
    "uniform sampler2D u_textures[32];\n" \
    "\n" \
    "void main()\n" \
    "{\n" \
    "    vec4 tex_color = texture(u_textures[v_tex_index], v_tex_coord);\n" \
    "    o_frag_color = tex_color * v_blend;\n" \
    "}\n"

#define ZFW_BUILTIN_CHAR_QUAD_VERT_SHADER_SRC \
    "#version 430 core\n" \
    "\n" \
    "layout (location = 0) in vec2 a_vert;\n" \
    "layout (location = 1) in vec2 a_tex_coord;\n" \
    "\n" \
    "out vec2 v_tex_coord;\n" \
    "\n" \
    "uniform vec2 u_pos;\n" \
    "uniform vec2 u_scale;\n" \
    "uniform mat4 u_proj;\n" \
    "\n" \
    "void main()\n" \
    "{\n" \
    "    mat4 model = mat4(\n" \
    "        vec4(u_scale.x, 0.0f, 0.0f, 0.0f),\n" \
    "        vec4(0.0f, u_scale.y, 0.0f, 0.0f),\n" \
    "        vec4(0.0f, 0.0f, 1.0f, 0.0f),\n" \
    "        vec4(u_pos.x, u_pos.y, 0.0f, 1.0f)\n" \
    "    );\n" \
    "    gl_Position = u_proj * model * vec4(a_vert, 0.0f, 1.0f);\n" \
    "\n" \
    "    v_tex_coord = a_tex_coord;\n" \
    "}\n"

#define ZFW_BUILTIN_CHAR_QUAD_FRAG_SHADER_SRC \
    "#version 430 core\n" \
    "\n" \
    "in vec2 v_tex_coord;\n" \
    "\n" \
    "out vec4 o_frag_color;\n" \
    "\n" \
    "uniform vec4 u_blend;\n" \
    "uniform sampler2D u_tex;\n" \
    "\n" \
    "void main()\n" \
    "{\n" \
    "    vec4 tex_color = texture(u_tex, v_tex_coord);\n" \
    "    o_frag_color = tex_color * u_blend;\n" \
    "}\n"

#define ZFW_BUILTIN_SPRITE_QUAD_SHADER_PROG_VERT_COUNT 14
#define ZFW_BUILTIN_CHAR_QUAD_SHADER_PROG_VERT_COUNT 4

typedef struct
{
    int tex_count;

    GLuint *gl_ids;
    zfw_vec_2d_i_t *sizes;
} zfw_user_tex_data_t;

typedef struct
{
    int prog_count;
    GLuint *gl_ids;
} zfw_user_shader_prog_data_t;

typedef struct
{
    int font_count;

    int *line_heights;

    font_char_hor_offs_t *chars_hor_offsets;
    font_char_vert_offs_t *chars_vert_offsets;

    font_char_hor_advance_t *chars_hor_advances;

    font_char_src_rect_t *chars_src_rects;

    font_char_kerning_t *chars_kernings;

    zfw_vec_2d_i_t *tex_sizes;
    GLuint *tex_gl_ids;
} zfw_user_font_data_t;

typedef struct
{
    GLuint sprite_quad_prog_gl_id;
    GLuint char_quad_prog_gl_id;
} zfw_builtin_shader_prog_data_t;

void zfw_gen_shader_prog(GLuint *const shader_prog_gl_id, const char *const vert_shader_src, const char *const frag_shader_src);
zfw_bool_t zfw_retrieve_user_asset_data_from_assets_file(zfw_user_tex_data_t *const tex_data, zfw_user_shader_prog_data_t *const shader_prog_data, zfw_user_font_data_t *const font_data, FILE *const assets_file_fs, zfw_mem_arena_t *const main_mem_arena);

#endif
