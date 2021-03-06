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

#ifndef GLASERLXX_H
#define GLASERLXX_H

#include "glaserl.h"

#include <memory>
#include <cstdarg>

namespace Glaserl {

class PProgram {
public:
    PProgram(const char *vertex_shader_src,
            const char *fragment_shader_src, va_list args)
        : d(NULL)
    {
        d = glaserl_program_newv(vertex_shader_src, fragment_shader_src, args);
    }

    ~PProgram()
    {
        glaserl_program_destroy(d);
    }

    void enable() { glaserl_program_enable(d); }
    void disable() { glaserl_program_disable(d); }

    GLint uniform_location(const char *uniform) {
        return glaserl_program_uniform_location(d, uniform);
    }

    size_t stride() { return d->stride; }

    glaserl_program_t *d;
};

typedef std::shared_ptr<PProgram> Program;

static inline Program
program(const char *vertex_shader_src, const char *fragment_shader_src, ...)
{
    va_list args;
    va_start(args, fragment_shader_src);
    return Program(new PProgram(vertex_shader_src, fragment_shader_src, args));
}

class PBuffer {
public:
    PBuffer()
        : d(glaserl_buffer_new())
    {
    }

    ~PBuffer()
    {
        glaserl_buffer_destroy(d);
    }

    void resize(size_t size) { glaserl_buffer_resize(d, size); }
    void append(void *data, size_t size) { glaserl_buffer_append(d, data, size); }
    void freeze() { glaserl_buffer_freeze(d); }
    size_t enable() { return glaserl_buffer_enable(d); }
    void disable() { glaserl_buffer_disable(d); }

    glaserl_buffer_t *d;
};

typedef std::shared_ptr<PBuffer> Buffer;

static inline Buffer
buffer()
{
    return Buffer(new PBuffer());
}

class PTexture {
private:
    PTexture(glaserl_texture_t *texture)
        : d(texture)
    {
    }

public:
    static PTexture *rgba(unsigned char *rgba, int width, int height)
    {
        return new PTexture(glaserl_texture_new(rgba, width, height));
    }

    static PTexture *rgb(unsigned char *rgb, int width, int height)
    {
        return new PTexture(glaserl_texture_new_rgb(rgb, width, height));
    }

    ~PTexture()
    {
        glaserl_texture_destroy(d);
    }

    void enable() { glaserl_texture_enable(d); }
    void disable() { glaserl_texture_disable(d); }
    int width() { return d->width; }
    int height() { return d->height; }
    void map_uv(float &u, float &v) { glaserl_texture_map_uv(d, &u, &v); }

    glaserl_texture_t *d;
};

typedef std::shared_ptr<PTexture> Texture;

static inline Texture
texture(unsigned char *rgba, int width, int height)
{
    return Texture(PTexture::rgba(rgba, width, height));
}

static inline Texture
texture_rgb(unsigned char *rgb, int width, int height)
{
    return Texture(PTexture::rgb(rgb, width, height));
}

class PFramebuffer {
public:
    PFramebuffer(int width, int height, bool rgba=false)
        : d(glaserl_framebuffer_new(width, height))
        , texture(rgba ? Glaserl::texture(nullptr, width, height)
                       : Glaserl::texture_rgb(nullptr, width, height))
    {
        glaserl_framebuffer_attach(d, texture->d);
    }

    ~PFramebuffer()
    {
        glaserl_framebuffer_destroy(d);
    }

    void enable() { glaserl_framebuffer_enable(d); }
    void disable() { glaserl_framebuffer_disable(d); }
    int width() { return d->width; }
    int height() { return d->height; }

    glaserl_framebuffer_t *d;
    Texture texture;
};

typedef std::shared_ptr<PFramebuffer> Framebuffer;

static inline Framebuffer
framebuffer(int width, int height)
{
    return Framebuffer(new PFramebuffer(width, height));
}

namespace Util {

static inline void
render_triangle_strip(Program &program, Buffer &buffer)
{
    glaserl_util_render_triangle_strip(program->d, buffer->d);
}

static inline void
default_blend()
{
    glaserl_util_default_blend();
}

static inline void
enable_scissor(bool enable=true)
{
    glaserl_util_enable_scissor(enable);
}

static inline void
set_scissor(int x, int y, int w, int h)
{
    glaserl_util_set_scissor(x, y, w, h);
}

}; // Util

}; /* Glaserl */

#endif /* GLASERLXX_H */
