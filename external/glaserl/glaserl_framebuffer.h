#ifndef GLASERL_FRAMEBUFFER_H
#define GLASERL_FRAMEBUFFER_H

#include "glaserl_gl.h"
#include "glaserl_texture.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct glaserl_framebuffer_t {
    int width;
    int height;

    GLuint id;

    GLint old_framebuffer;
    GLint old_viewport[4];
};

typedef struct glaserl_framebuffer_t glaserl_framebuffer_t;

glaserl_framebuffer_t *
glaserl_framebuffer_new(int width, int height);

void
glaserl_framebuffer_attach(glaserl_framebuffer_t *framebuffer,
        glaserl_texture_t *texture);

void
glaserl_framebuffer_enable(glaserl_framebuffer_t *framebuffer);

void
glaserl_framebuffer_disable(glaserl_framebuffer_t *framebuffer);

void
glaserl_framebuffer_destroy(glaserl_framebuffer_t *framebuffer);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* GLASERL_FRAMEBUFFER_H */
