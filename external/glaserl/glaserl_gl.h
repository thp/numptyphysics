#ifndef GLASERL_GL_H
#define GLASERL_GL_H

#include <stdlib.h>

#if defined(USE_OPENGL_ES)
#    include <GLES2/gl2.h>
#    define GLASERL_GLSL_PRECISION_INFO "precision mediump float;\n"
#elif defined(__APPLE__)
#    include <OpenGL/gl.h>
#else
#    define GL_GLEXT_PROTOTYPES
#    include <GL/gl.h>
#    include <GL/glext.h>
#endif

#if !defined(GLASERL_GLSL_PRECISION_INFO)
#define GLASERL_GLSL_PRECISION_INFO ""
#endif /* !defined(GLASERL_GLSL_PRECISION_INFO) */

#endif /* GLASERL_GL_H */
