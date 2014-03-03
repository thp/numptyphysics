#include "glaserl_buffer.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

glaserl_buffer_t *
glaserl_buffer_new()
{
    glaserl_buffer_t *buffer = (glaserl_buffer_t *)calloc(sizeof(glaserl_buffer_t), 1);

    buffer->buffer = 0;
    buffer->size = 0;
    buffer->offset = 0;
    buffer->frozen = 0;

    glaserl_buffer_resize(buffer, 1024);

    glGenBuffers(1, &(buffer->id));

    return buffer;
}

void
glaserl_buffer_resize(glaserl_buffer_t *buffer, size_t size)
{
    if (buffer->frozen) {
        assert(0);
        return;
    }

    buffer->size = size;
    buffer->buffer = (char *)realloc(buffer->buffer, size);
}

void
glaserl_buffer_append(glaserl_buffer_t *buffer, void *data, size_t size)
{
    if (buffer->frozen) {
        assert(0);
        return;
    }

    if (buffer->offset + size > buffer->size) {
        size_t new_size = buffer->size;
        while (buffer->offset + size > new_size) {
            new_size *= 2;
        }
        glaserl_buffer_resize(buffer, new_size);
    }

    memcpy(buffer->buffer + buffer->offset, data, size);
    buffer->offset += size;
}

void
glaserl_buffer_freeze(glaserl_buffer_t *buffer)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);
    glBufferData(GL_ARRAY_BUFFER, buffer->offset, buffer->buffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    buffer->frozen = 1;
}

size_t
glaserl_buffer_enable(glaserl_buffer_t *buffer)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer->id);

    size_t size = buffer->offset;

    if (!buffer->frozen) {
        glBufferData(GL_ARRAY_BUFFER, buffer->offset, buffer->buffer, GL_STREAM_DRAW);
        buffer->offset = 0;
    }

    return size;
}

void
glaserl_buffer_disable(glaserl_buffer_t *buffer)
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void
glaserl_buffer_destroy(glaserl_buffer_t *buffer)
{
    glDeleteBuffers(1, &(buffer->id));

    free(buffer->buffer);
    free(buffer);
}
