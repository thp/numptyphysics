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
