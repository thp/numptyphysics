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

float *
glaserl_matrix_data(glaserl_matrix_t *matrix);

void
glaserl_matrix_destroy(glaserl_matrix_t *matrix);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* GLASERL_MATRIX_H */
