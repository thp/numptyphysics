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

#ifndef GLASERL_PROGRAM_H
#define GLASERL_PROGRAM_H

#include "glaserl_gl.h"

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Maximum numbers of attributes and uniforms
#define GLASERL_PROGRAM_MAX_ATTRIBUTES 10
#define GLASERL_PROGRAM_MAX_UNIFORMS 10

struct glaserl_program_attrib_t {
    const char *name;
    int size; // in number of floats
    GLint location;
};

typedef struct glaserl_program_attrib_t glaserl_program_attrib_t;

struct glaserl_program_uniform_t {
    const char *name;
    GLint location;
};

typedef struct glaserl_program_uniform_t glaserl_program_uniform_t;

struct glaserl_program_t {
    glaserl_program_attrib_t attributes[GLASERL_PROGRAM_MAX_ATTRIBUTES];
    glaserl_program_uniform_t uniforms[GLASERL_PROGRAM_MAX_UNIFORMS];

    GLuint program;
    size_t stride;
};

typedef struct glaserl_program_t glaserl_program_t;

glaserl_program_t *
glaserl_program_new(const char *vertex_shader_src,
        const char *fragment_shader_src, ...);

glaserl_program_t *
glaserl_program_newv(const char *vertex_shader_src,
        const char *fragment_shader_src, va_list args);

void
glaserl_program_enable(glaserl_program_t *program);

void
glaserl_program_disable(glaserl_program_t *program);

GLint
glaserl_program_uniform_location(glaserl_program_t *program,
        const char *uniform);

void
glaserl_program_destroy(glaserl_program_t *program);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* GLASERL_PROGRAM_H */
