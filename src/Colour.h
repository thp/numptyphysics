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

#ifndef NUMPTYPHYSICS_COLOUR_H
#define NUMPTYPHYSICS_COLOUR_H

namespace NP {
namespace Colour {

extern const int values[];
extern const int count;

inline int toIndex(int colour)
{
    for (int i=0; i<count; i++) {
        if (values[i] == colour) {
            return i;
        }
    }

    return -1;
}

static const int RED = 0xb80000;
static const int YELLOW = 0xeec900;
static const int DEFAULT = 0x000077;

static const int DEFAULT_BG = 0x303030;
static const int DEFAULT_FG = 0xf8fcf8;
static const int BUTTON_BG = 0x383c38;
static const int SELECTED_BG = 0x404040;
static const int TL_BORDER = 0x909490;
static const int BR_BORDER = 0x182018;

}; /* Colour */
}; /* NP */

#endif /* NUMPTYPHYSICS_COLOUR_H */
