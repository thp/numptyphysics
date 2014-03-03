#include "glaserl_texture.h"

glaserl_texture_t *
glaserl_texture_new(unsigned char *rgba, int width, int height)
{
    glaserl_texture_t *texture = (glaserl_texture_t *)calloc(sizeof(glaserl_texture_t), 1);
    glGenTextures(1, &texture->id);
    glaserl_texture_enable(texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    int w = 1;
    while (w < width) w *= 2;
    int h = 1;
    while (h < height) h *= 2;

    texture->width = width;
    texture->height = height;

    texture->subwidth = (float)width / (float)w;
    texture->subheight = (float)height / (float)h;
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
