/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2015 Thomas Perl <m@thp.io>
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

#include "petals_log.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef ANDROID
#include <android/log.h>
#endif

#if __APPLE__
#  include "TargetConditionals.h"
#  if TARGET_IPHONE_SIMULATOR
#    define PETALS_LOG_COLOR false
#  elif TARGET_OS_IPHONE
#    define PETALS_LOG_COLOR false
#  else
#    define PETALS_LOG_COLOR true
#  endif
#else
#  define PETALS_LOG_COLOR true
#endif

static long (*g_ticks)() = nullptr;
static std::string (*g_format)(const char *fmt, ...) = nullptr;

namespace PetalsLog {

void
init(long (*ticks)(), std::string (*format)(const char *fmt, ...))
{
    g_ticks = ticks;
    g_format = format;
}

void
log(enum Level level, const char *filename, const char *function,
        int lineno, const char *fmt, ...)
{
    const char *levelname = "";
    int color = 0;

    switch (level) {
        case LEVEL_DEBUG:
            break;
        case LEVEL_INFO:
            levelname = " INFO";
            color = 36; // cyan
            break;
        case LEVEL_WARNING:
            levelname = " WARNING";
            color = 33; // yellow
            break;
        case LEVEL_ERROR:
            levelname = " ERROR";
            color = 31; // red
            break;
        case LEVEL_FATAL:
            levelname = " FATAL";
            color = 31; // red
            break;
    }

    const char *fn = strrchr(filename, '/');
    if (fn == nullptr) {
        fn = filename;
    } else {
        fn++;
    }

    std::string sfmt;
    if (g_format == nullptr || g_ticks == nullptr) {
        sfmt = "<LOGGING NOT INITIALIZED (forgot to call PetalsLog::init()?)>\n";
    } else if (PETALS_LOG_COLOR) {
        sfmt = ::g_format("[%5dms] [%s:%d%s] \x1b[%dm%s\x1b[0m%c",
            ::g_ticks(), fn, lineno, levelname, color, fmt,
            ((fmt[strlen(fmt)-1] == '\n') ? '\0' : '\n'));
    } else {
        sfmt = ::g_format("[%5dms] [%s:%d%s] %s%c",
                            ::g_ticks(), fn, lineno, levelname, fmt,
                            ((fmt[strlen(fmt)-1] == '\n') ? '\0' : '\n'));
    }

    va_list args;
    va_start(args, fmt);
#ifdef ANDROID
    __android_log_vprint(ANDROID_LOG_DEBUG, "LOG_TAG", sfmt.c_str(), args);
#else
    vfprintf(stderr, sfmt.c_str(), args);
#endif

    if (level == LEVEL_FATAL) {
        exit(1);
    }
}

}; /* namespace PetalsLog */
