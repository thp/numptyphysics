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

class PMatrix {
public:
    PMatrix()
        : d(glaserl_matrix_new())
    {
    }

    ~PMatrix()
    {
        glaserl_matrix_destroy(d);
    }

    void identity() { glaserl_matrix_identity(d); }
    void ortho(float left, float right, float bottom, float top,
            float zNear, float zFar)
    {
        glaserl_matrix_ortho(d, left, right, bottom, top, zNear, zFar);
    }
    float *data() { return glaserl_matrix_data(d); }

    glaserl_matrix_t *d;
};

typedef std::shared_ptr<PMatrix> Matrix;

class PTexture {
public:
    PTexture(unsigned char *rgba, int width, int height)
        : d(glaserl_texture_new(rgba, width, height))
    {
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

static inline Program
program(const char *vertex_shader_src, const char *fragment_shader_src, ...)
{
    va_list args;
    va_start(args, fragment_shader_src);
    return Program(new PProgram(vertex_shader_src, fragment_shader_src, args));
}

static inline Buffer
buffer()
{
    return Buffer(new PBuffer());
}

static inline Matrix
matrix()
{
    return Matrix(new PMatrix());
}

static inline Texture
texture(unsigned char *rgba, int width, int height)
{
    return Texture(new PTexture(rgba, width, height));
}

namespace Util {

static inline void
render_triangle_strip(Program &program, Buffer &buffer)
{
    glaserl_util_render_triangle_strip(program->d, buffer->d);
}

static inline void
load_matrix(Program &program, const char *uniform, Matrix &matrix)
{
    glaserl_util_load_matrix(program->d, uniform, matrix->d);
}

static inline void
default_blend()
{
    glaserl_util_default_blend();
}

}; // Util

}; /* Glaserl */

#endif /* GLASERLXX_H */
