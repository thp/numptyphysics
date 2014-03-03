#ifndef GLASERL_UTIL_H
#define GLASERL_UTIL_H

#include "glaserl_program.h"
#include "glaserl_buffer.h"
#include "glaserl_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void
glaserl_util_render_triangle_strip(glaserl_program_t *program, glaserl_buffer_t *buffer);

void
glaserl_util_load_matrix(glaserl_program_t *program, const char *uniform, glaserl_matrix_t *matrix);

void
glaserl_util_default_blend();

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* GLASERL_UTIL_H */
