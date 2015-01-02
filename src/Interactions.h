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

#ifndef NUMPTYPHYSICS_INTERACTIONS_H
#define NUMPTYPHYSICS_INTERACTIONS_H

#include <string>
#include <map>

namespace NP {

class Interactions {
public:
    Interactions();
    ~Interactions();

    bool handle(int color);
    void clear();

    bool parse(const std::string &line);
    bool add(const std::string &color, const std::string &action);
    std::string serialize();

private:
    std::map<int,std::string> m_interactions;
};

}; /* namespace NP */

#endif /* NUMPTYPHYSICS_INTERACTIONS_H */
