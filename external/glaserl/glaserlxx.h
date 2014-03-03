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

    GLint uniform(int index) { return d->uniforms[index].location; }

    size_t stride() { return d->stride; }

private:
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

private:
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

private:
    glaserl_matrix_t *d;
};

typedef std::shared_ptr<PMatrix> Matrix;

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

}; /* Glaserl */

#endif /* GLASERLXX_H */
