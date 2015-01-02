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

#ifndef GLASERL_MATRIX_H
#define GLASERL_MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct glaserl_matrix_t {
    float d[4*4];
};

typedef struct glaserl_matrix_t glaserl_matrix_t;

glaserl_matrix_t *
glaserl_matrix_new();

void
glaserl_matrix_identity(glaserl_matrix_t *matrix);

void
glaserl_matrix_ortho(glaserl_matrix_t *matrix, float left, float right,
        float bottom, float top, float zNear, float zFar);

void
glaserl_matrix_rotate_2d(glaserl_matrix_t *matrix, float angle);

float *
glaserl_matrix_data(glaserl_matrix_t *matrix);

void
glaserl_matrix_destroy(glaserl_matrix_t *matrix);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* GLASERL_MATRIX_H */
