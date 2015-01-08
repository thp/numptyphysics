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

#include "glaserl_texture.h"

static glaserl_texture_t *
_glaserl_texture_new(int width, int height, int *w, int *h)
{
    glaserl_texture_t *texture = (glaserl_texture_t *)calloc(sizeof(glaserl_texture_t), 1);
    glGenTextures(1, &texture->id);
    glaserl_texture_enable(texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    while (*w < width) *w *= 2;
    while (*h < height) *h *= 2;

    texture->width = width;
    texture->height = height;

    texture->subwidth = (float)width / (float)*w;
    texture->subheight = (float)height / (float)*h;

    return texture;
}

glaserl_texture_t *
glaserl_texture_new(unsigned char *rgba, int width, int height)
{
    int w = 1;
    int h = 1;
    glaserl_texture_t *texture = _glaserl_texture_new(width, height, &w, &h);

    texture->format = GLASERL_TEXTURE_RGBA;
    unsigned char *blackness = calloc(sizeof(unsigned char), w * h * 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, blackness);
    free(blackness);

    if (rgba) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                        GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    }

    glaserl_texture_disable(texture);
    return texture;
}

glaserl_texture_t *
glaserl_texture_new_rgb(unsigned char *rgb, int width, int height)
{
    int w = 1;
    int h = 1;
    glaserl_texture_t *texture = _glaserl_texture_new(width, height, &w, &h);

    texture->format = GLASERL_TEXTURE_RGB;
    unsigned char *blackness = calloc(sizeof(unsigned char), w * h * 3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, blackness);
    free(blackness);

    if (rgb) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                        GL_RGB, GL_UNSIGNED_BYTE, rgb);
    }

    glaserl_texture_disable(texture);
    return texture;
}

void
glaserl_texture_enable(glaserl_texture_t *texture)
{
    glBindTexture(GL_TEXTURE_2D, texture->id);
}

void
glaserl_texture_disable(glaserl_texture_t *texture)
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void
glaserl_texture_map_uv(glaserl_texture_t *texture, float *u, float *v)
{
    *u *= texture->subwidth;
    *v *= texture->subheight;
}

void
glaserl_texture_destroy(glaserl_texture_t *texture)
{
    glDeleteTextures(1, &texture->id);
    free(texture);
}
