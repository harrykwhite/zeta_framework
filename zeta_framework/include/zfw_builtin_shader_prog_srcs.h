#ifndef __ZFW_BUILTIN_SHADER_PROG_SRCS__
#define __ZFW_BUILTIN_SHADER_PROG_SRCS__

#define ZFW_BUILTIN_TEXTURED_RECT_VERT_SHADER_SRC \
    "#version 430 core\n" \
    "\n" \
    "layout (location = 0) in vec2 a_vert;\n" \
    "layout (location = 1) in vec2 a_pos;\n" \
    "layout (location = 2) in vec2 a_size;\n" \
    "layout (location = 3) in float a_rot;\n" \
    "layout (location = 4) in float a_depth;\n" \
    "layout (location = 5) in float a_tex_index;\n" \
    "layout (location = 6) in vec2 a_tex_coord;\n" \
    "layout (location = 7) in float a_opacity;\n" \
    "\n" \
    "out flat int v_tex_index;\n" \
    "out vec2 v_tex_coord;\n" \
    "out flat float v_opacity;\n" \
    "\n" \
    "uniform mat4 u_view;\n" \
    "uniform mat4 u_proj;\n" \
    "\n" \
    "void main()\n" \
    "{\n" \
    "    float rot_cos = cos(a_rot);\n" \
    "    float rot_sin = sin(a_rot);\n" \
    "\n" \
    "    mat4 model = mat4(\n" \
    "        vec4(a_size.x * rot_cos, a_size.x * rot_sin, 0.0f, 0.0f),\n" \
    "        vec4(a_size.y * -rot_sin, a_size.y * rot_cos, 0.0f, 0.0f),\n" \
    "        vec4(0.0f, 0.0f, 1.0f, 0.0f),\n" \
    "        vec4(a_pos.x, a_pos.y, 0.0f, 1.0f)\n" \
    "    );\n" \
    "\n" \
    "    gl_Position = u_proj * u_view * model * vec4(a_vert, a_depth, 1.0f);\n" \
    "\n" \
    "    v_tex_index = int(a_tex_index);\n" \
    "    v_tex_coord = a_tex_coord;\n" \
    "    v_opacity = a_opacity;\n" \
    "}\n"

#define ZFW_BUILTIN_TEXTURED_RECT_FRAG_SHADER_SRC \
    "#version 430 core\n" \
    "\n" \
    "in flat int v_tex_index;\n" \
    "in vec2 v_tex_coord;\n" \
    "in flat float v_opacity;\n" \
    "\n" \
    "out vec4 o_frag_color;\n" \
    "\n" \
    "uniform sampler2D u_textures[32];\n" \
    "\n" \
    "void main()\n" \
    "{\n" \
    "    o_frag_color = vec4(texture(u_textures[v_tex_index], v_tex_coord).rgb, texture(u_textures[v_tex_index], v_tex_coord).a * v_opacity);\n" \
    "}\n"

#define ZFW_BUILTIN_DEFAULT_RENDER_LAYER_VERT_SHADER_SRC \
    "#version 430 core\n" \
    "\n" \
    "layout (location = 0) in vec2 a_vert;\n" \
    "layout (location = 1) in vec2 a_tex_coord;\n" \
    "\n" \
    "out vec2 v_tex_coord;\n" \
    "uniform mat4 u_view;\n" \
    "\n" \
    "void main()\n" \
    "{\n" \
    "    gl_Position = u_view * vec4(a_vert, 0.0f, 1.0f);\n" \
    "    v_tex_coord = a_tex_coord;\n" \
    "}\n"

#define ZFW_BUILTIN_DEFAULT_RENDER_LAYER_FRAG_SHADER_SRC \
    "#version 430 core\n" \
    "\n" \
    "in vec2 v_tex_coord;\n" \
    "out vec4 o_frag_color;\n" \
    "uniform sampler2D u_texture;\n" \
    "\n" \
    "void main()\n" \
    "{\n" \
    "    o_frag_color = texture(u_texture, v_tex_coord);\n" \
    "}\n"

#endif
