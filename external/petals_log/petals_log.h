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

#ifndef PETALSLOG_H
#define PETALSLOG_H

#include <string>

namespace PetalsLog {

enum Level {
    LEVEL_DEBUG = 0,
    LEVEL_INFO,
    LEVEL_WARNING,
    LEVEL_ERROR,
    LEVEL_FATAL,
};

void
init(long (*ticks)(), std::string (*format)(const char *fmt, ...));

void
log(enum Level level, const char *filename, const char *function,
        int lineno, const char *fmt, ...);

}; /* namespace PetalsLog */

#if defined(RELEASE_BUILD)
#  define PETALS_LOG(level, ...)
#  define LOG_FATAL(...) exit(1)
#else
#  define PETALS_LOG(level, ...) PetalsLog::log(level, __FILE__, __func__, __LINE__, __VA_ARGS__)
#  define LOG_FATAL(...) PETALS_LOG(PetalsLog::LEVEL_FATAL, __VA_ARGS__)
#endif /* defined(RELEASE_BUILD) */

#define LOG_DEBUG(...) PETALS_LOG(PetalsLog::LEVEL_DEBUG, __VA_ARGS__)
#define LOG_INFO(...) PETALS_LOG(PetalsLog::LEVEL_INFO, __VA_ARGS__)
#define LOG_WARNING(...) PETALS_LOG(PetalsLog::LEVEL_WARNING, __VA_ARGS__)
#define LOG_ERROR(...) PETALS_LOG(PetalsLog::LEVEL_ERROR, __VA_ARGS__)

#define LOG_NOTREACHED PETALS_FATAL("Unreachable code")

#endif /* PETALSLOG_H */
