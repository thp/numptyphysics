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

#ifndef GLASERL_UTIL_H
#define GLASERL_UTIL_H

#include "glaserl_program.h"
#include "glaserl_buffer.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void
glaserl_util_render_triangle_strip(glaserl_program_t *program, glaserl_buffer_t *buffer);

void
glaserl_util_default_blend();

void
glaserl_util_enable_scissor(bool enable);

void
glaserl_util_set_scissor(int x, int y, int w, int h);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* GLASERL_UTIL_H */
