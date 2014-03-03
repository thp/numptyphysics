#ifndef GLASERL_TEXTURE_H
#define GLASERL_TEXTURE_H

#include "glaserl_gl.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum glaserl_texture_format_t {
    GLASERL_TEXTURE_RGBA,
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
