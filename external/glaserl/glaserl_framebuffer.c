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

#include "glaserl_framebuffer.h"

#include <stdio.h>
#include <stdlib.h>

glaserl_framebuffer_t *
glaserl_framebuffer_new(int width, int height)
{
    glaserl_framebuffer_t *framebuffer = (glaserl_framebuffer_t *)calloc(sizeof(glaserl_framebuffer_t), 1);
    framebuffer->width = width;
    framebuffer->height = height;
    glGenFramebuffers(1, &framebuffer->id);
    return framebuffer;
}

void
glaserl_framebuffer_attach(glaserl_framebuffer_t *framebuffer,
        glaserl_texture_t *texture)
{
    glaserl_framebuffer_enable(framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, texture->id, 0);
    glaserl_framebuffer_disable(framebuffer);
}

void
glaserl_framebuffer_enable(glaserl_framebuffer_t *framebuffer)
{
    GLint current = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current);
    if (current != framebuffer->id) {
        framebuffer->old_framebuffer = current;
        glGetIntegerv(GL_VIEWPORT, framebuffer->old_viewport);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->id);
        glViewport(0, 0, framebuffer->width, framebuffer->height);
    }
}

void
glaserl_framebuffer_disable(glaserl_framebuffer_t *framebuffer)
{
    GLint current = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current);
    if (current == framebuffer->id) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->old_framebuffer);
        glViewport(framebuffer->old_viewport[0], framebuffer->old_viewport[1],
                framebuffer->old_viewport[2], framebuffer->old_viewport[3]);
    }
}

void
glaserl_framebuffer_destroy(glaserl_framebuffer_t *framebuffer)
{
    glDeleteFramebuffers(1, &framebuffer->id);
    free(framebuffer);
}
