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

#ifndef I18N_H
#define I18N_H

#include "Common.h"
#include "thp_format.h"

#include <string>
#include <map>

class Tr : public thp::stringable {
public:
    explicit Tr(const char *key) : key(key) {}

    operator bool() const;
    virtual const char *c_str() const;

    static Tr copy(const std::string &s);
    static Tr defer(std::function<std::string()> fmt);

    static void load(const std::string &filename);

private:
    const char *key;
    std::string buf;
    std::function<std::string()> fmt;
};

#define TR_DEFERRED(x) Tr::defer([=] () { return (x); })

#endif /* I18N_H */
