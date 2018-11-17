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

#ifndef GLASERL_GL_H
#define GLASERL_GL_H

#include <stdlib.h>

#if defined(USE_OPENGL_ES)
#    include <GLES2/gl2.h>
#    define GLASERL_GLSL_PRECISION_INFO "precision mediump float;\n"
#elif defined(__APPLE__)
#    include <OpenGL/gl.h>
#elif defined(__WIN32__)
#    include "gl.h"
#else
#    define GL_GLEXT_PROTOTYPES
#    include <GL/gl.h>
#    include <GL/glext.h>
#endif

#if !defined(GLASERL_GLSL_PRECISION_INFO)
#define GLASERL_GLSL_PRECISION_INFO ""
#endif /* !defined(GLASERL_GLSL_PRECISION_INFO) */

#endif /* GLASERL_GL_H */
