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

#include "Interactions.h"

#include "thp_format.h"
#include "petals_log.h"

#include <sstream>
#include <cctype>

namespace NP {

Interactions::Interactions()
    : m_interactions()
{
}

Interactions::~Interactions()
{
}

bool
Interactions::handle(int color)
{
    auto it = m_interactions.find(color);
    if (it != m_interactions.end()) {
        LOG_INFO("Interaction event: '%s'", it->second.c_str());
        return true;
    }

    return false;
}

void
Interactions::clear()
{
}

bool
Interactions::parse(const std::string &line)
{
    // Interaction: 123 = main_menu
    //             ^^^^^^^^^^^^^^^^
    const char *tmp = line.c_str();
    while (*tmp && ::isspace(*tmp)) {
        tmp++;
    }
    if (*tmp < '0' || *tmp > '9') {
        return false;
    }

    int color = atoi(tmp);
    while (*tmp && *tmp != '=') {
        tmp++;
    }
    if (*tmp != '=') {
        return false;
    }
    tmp++;
    while (*tmp && ::isspace(*tmp)) {
        tmp++;
    }
    if (!*tmp) {
        return false;
    }
    std::string action(tmp);

    m_interactions[color] = action;
    return true;
}

bool
Interactions::add(const std::string &color, const std::string &action)
{
    m_interactions[atoi(color.c_str())] = action;
    return true;
}

std::string
Interactions::serialize()
{
    std::stringstream s;

    for (auto &interaction: m_interactions) {
        s << thp::format("<np:interaction np:color=\"%d\" np_action=\"%s\" />",
                         interaction.first, interaction.second.c_str()) << std::endl;
    }

    return s.str();
}

}; /* namespace NP */
