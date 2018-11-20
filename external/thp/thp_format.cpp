/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2014, 2015 Thomas Perl <m@thp.io>
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "thp_format.h"

#include <string>
#include <cstdarg>
#include <cstdlib>


std::string
thp::format(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char *tmp;
    vasprintf(&tmp, fmt, ap);
    std::string result = tmp;
    free(tmp);

    return result;
}

std::string
thp::format(const std::string &fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char *tmp;
    vasprintf(&tmp, fmt.c_str(), ap);
    std::string result = tmp;
    free(tmp);

    return result;
}

std::string
thp::format(const thp::stringable &fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char *tmp;
    vasprintf(&tmp, fmt.c_str(), ap);
    std::string result = tmp;
    free(tmp);

    return result;
}
