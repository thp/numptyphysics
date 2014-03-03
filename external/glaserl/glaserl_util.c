#include "glaserl_util.h"

void
glaserl_util_render_triangle_strip(glaserl_program_t *program, glaserl_buffer_t *buffer)
{
    size_t size = glaserl_buffer_enable(buffer);
    glaserl_program_enable(program);
    glaserl_buffer_disable(buffer);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, size / program->stride);

    glaserl_program_disable(program);
}

void
glaserl_util_load_matrix(glaserl_program_t *program, const char *uniform, glaserl_matrix_t *matrix)
{
    GLint location = glaserl_program_uniform_location(program, uniform);
    glaserl_program_enable(program);

    glUniformMatrix4fv(location, 1, GL_FALSE, glaserl_matrix_data(matrix));

    glaserl_program_disable(program);
}

void
glaserl_util_default_blend()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
