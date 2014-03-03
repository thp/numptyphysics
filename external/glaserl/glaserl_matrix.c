#include "glaserl_matrix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

glaserl_matrix_t *
glaserl_matrix_new()
{
    glaserl_matrix_t *matrix;

    matrix = (glaserl_matrix_t *)calloc(sizeof(glaserl_matrix_t), 1);

    glaserl_matrix_identity(matrix);

    return matrix;
}

void
glaserl_matrix_identity(glaserl_matrix_t *matrix)
{
    int x, y;

    for (y=0; y<4; y++) {
        for (x=0; x<4; x++) {
            matrix->d[y*4+x] = (x == y) ? 1.f : 0.f;
        }
    }
}

static void
_glaserl_matrix_transpose_(float *d)
{
    int x, y;

    for (y=0; y<4; y++) {
        for (x=y+1; x<4; x++) {
            float tmp = d[y*4+x];
            d[y*4+x] = d[x*4+y];
            d[x*4+y] = tmp;
        }
    }
}

static void
_glaserl_matrix_mult_(float *a, float *b, float *c)
{
    int x, y, z;

    for (y=0; y<4; y++) {
        for (x=0; x<4; x++) {
            c[y*4+x] = 0.f;
            for (z=0; z<4; z++) {
                c[y*4+x] += a[z*4+x] * b[y*4+z];
            }
        }
    }
}

static void
_glaserl_matrix_set_(float *dst, float *src)
{
    int i;

    for (i=0; i<16; i++) {
        dst[i] = src[i];
    }
}

void
glaserl_matrix_ortho(glaserl_matrix_t *matrix, float left, float right,
        float bottom, float top, float zNear, float zFar)
{
    float tx = - (right + left) / (right - left);
    float ty = - (top + bottom) / (top - bottom);
    float tz = - (zFar + zNear) / (zFar - zNear);

    float o[] = {
        2.f / (right - left), 0.f, 0.f, tx,
        0.f, 2.f / (top - bottom), 0.f, ty,
        0.f, 0.f, -2.f / (zFar - zNear), tz,
        0.f, 0.f, 0.f, 1.f,
    };
    _glaserl_matrix_transpose_(o);

    float tmp[16];
    _glaserl_matrix_mult_(matrix->d, o, tmp);
    _glaserl_matrix_set_(matrix->d, tmp);
}

float *
glaserl_matrix_data(glaserl_matrix_t *matrix)
{
    return matrix->d;
}

void
glaserl_matrix_destroy(glaserl_matrix_t *matrix)
{
    free(matrix);
}
