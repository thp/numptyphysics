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

#include "glaserl_program.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static GLuint
make_shader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);

    const char *shader_src[] = { GLASERL_GLSL_PRECISION_INFO, source };
    glShaderSource(shader, 2, shader_src, 0);

    glCompileShader(shader);

    GLint result = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (!result) {
        GLint size = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
        char *tmp = (char *)malloc(size);
        glGetShaderInfoLog(shader, size, NULL, tmp);
        printf("Failed to link shader: %s\n", tmp);
        free(tmp);
        exit(0);
    }

    return shader;
}

static GLuint
make_program(const char *vertex_shader, const char *fragment_shader)
{
    GLuint program = glCreateProgram();

    glAttachShader(program, make_shader(GL_VERTEX_SHADER, vertex_shader));
    glAttachShader(program, make_shader(GL_FRAGMENT_SHADER, fragment_shader));

    glLinkProgram(program);

    GLint result = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (!result) {
        printf("Failed to link program!\n");
        exit(0);
    }

    return program;
}

glaserl_program_t *
glaserl_program_new(const char *vertex_shader_src,
        const char *fragment_shader_src, ...)
{
    va_list args;
    va_start(args, fragment_shader_src);

    return glaserl_program_newv(vertex_shader_src, fragment_shader_src, args);
}

glaserl_program_t *
glaserl_program_newv(const char *vertex_shader_src,
        const char *fragment_shader_src, va_list args)
{
    glaserl_program_t *program;
    int i;

    program = (glaserl_program_t *)calloc(sizeof(glaserl_program_t), 1);
    program->program = make_program(vertex_shader_src, fragment_shader_src);
    program->stride = 0;

    // Attributes
    i = 0;
    for (;;) {
        const char *name = va_arg(args, const char *);
        if (name == NULL) {
            break;
        }

        int size = va_arg(args, int);

        program->attributes[i].name = strdup(name);
        program->attributes[i].size = size;
        program->attributes[i].location =
            glGetAttribLocation(program->program, name);
        program->stride += size * sizeof(float);
        i++;
    }

    // Uniforms
    i = 0;
    for (;;) {
        const char *name = va_arg(args, const char *);
        if (name == NULL) {
            break;
        }

        program->uniforms[i].name = strdup(name);
        program->uniforms[i].location =
            glGetUniformLocation(program->program, name);
        i++;
    }

    va_end(args);

    return program;
}

void
glaserl_program_enable(glaserl_program_t *program)
{
    glUseProgram(program->program);

    glaserl_program_attrib_t *attr = program->attributes;
    size_t offset = 0;
    float *fbuffer = 0;
    while (attr && attr->name) {
        glEnableVertexAttribArray(attr->location);
        glVertexAttribPointer(attr->location, attr->size,
                GL_FLOAT, GL_FALSE, program->stride,
                fbuffer + offset);
        offset += attr->size;
        attr++;
    }
}

void
glaserl_program_disable(glaserl_program_t *program)
{
    glaserl_program_attrib_t *attr = program->attributes;
    while (attr && attr->name) {
        glDisableVertexAttribArray(attr->location);
        attr++;
    }

    glUseProgram(0);
}

GLint
glaserl_program_uniform_location(glaserl_program_t *program,
        const char *uniform)
{
    glaserl_program_uniform_t *unif = program->uniforms;
    while (unif && unif->name) {
        if (strcmp(unif->name, uniform) == 0) {
            return unif->location;
        }
        unif++;
    }

    return -1;
}

void
glaserl_program_destroy(glaserl_program_t *program)
{
    // TODO
}
