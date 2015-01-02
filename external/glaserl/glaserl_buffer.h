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

#ifndef GLASERL_BUFFER_H
#define GLASERL_BUFFER_H

#include "glaserl_gl.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct glaserl_buffer_t {
    char *buffer;
    size_t size;
    size_t offset;

    GLuint id;
    int frozen;
};

typedef struct glaserl_buffer_t glaserl_buffer_t;

glaserl_buffer_t *
glaserl_buffer_new();

void
glaserl_buffer_resize(glaserl_buffer_t *buffer, size_t size);

void
glaserl_buffer_append(glaserl_buffer_t *buffer, void *data, size_t size);

void
glaserl_buffer_freeze(glaserl_buffer_t *buffer);

size_t
glaserl_buffer_enable(glaserl_buffer_t *buffer);

void
glaserl_buffer_disable(glaserl_buffer_t *buffer);

void
glaserl_buffer_destroy(glaserl_buffer_t *buffer);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* GLASERL_BUFFER_H */
