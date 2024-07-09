#include <zfw.h>

#include <stdlib.h>

void zfw_gen_shader_prog(GLuint *const shader_prog_gl_id, const char *const vert_shader_src, const char *const frag_shader_src)
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
