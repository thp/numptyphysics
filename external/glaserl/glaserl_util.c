/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2014 Thomas Perl <m@thp.io>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

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
glaserl_util_default_blend()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void
glaserl_util_enable_scissor(bool enable)
{
    if (enable) {
        glEnable(GL_SCISSOR_TEST);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
}

void
glaserl_util_set_scissor(int x, int y, int w, int h)
{
    glScissor(x, y, w, h);
}
