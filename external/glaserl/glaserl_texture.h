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

#ifndef GLASERL_TEXTURE_H
#define GLASERL_TEXTURE_H

#include "glaserl_gl.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum glaserl_texture_format_t {
    GLASERL_TEXTURE_RGBA,
    GLASERL_TEXTURE_RGB,
};

struct glaserl_texture_t {
    GLuint id;

    int width;
    int height;

    float subwidth;
    float subheight;

    enum glaserl_texture_format_t format;
};

typedef struct glaserl_texture_t glaserl_texture_t;

glaserl_texture_t *
glaserl_texture_new(unsigned char *rgba, int width, int height);

glaserl_texture_t *
glaserl_texture_new_rgb(unsigned char *rgb, int width, int height);

void
glaserl_texture_enable(glaserl_texture_t *texture);

void
glaserl_texture_disable(glaserl_texture_t *texture);

void
glaserl_texture_map_uv(glaserl_texture_t *texture, float *u, float *v);

void
glaserl_texture_destroy(glaserl_texture_t *texture);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* GLASERL_TEXTURE_H */
