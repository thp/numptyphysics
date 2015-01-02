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

#include "Colour.h"

namespace NP {
namespace Colour {

const int values[] = {
    RED,
    YELLOW,
    DEFAULT,
  0x108710, //green
  0x101010, //black
  0x8b4513, //brown
  0x87cefa, //lightblue
  0xee6aa7, //pink
  0xb23aee, //purple
  0x00fa9a, //lightgreen
  0xff7f00, //orange
  0x6c7b8b, //grey
};

const int count = (sizeof(values)/sizeof(values[0]));

}; /* Colour */
}; /* NP */
