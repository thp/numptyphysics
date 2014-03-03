/*
 * This file is part of NumptyPhysics
 * Copyright (C) 2008 Tim Edmonds
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
 *
 */

#include "Config.h"


Rect FULLSCREEN_RECT( 0, 0, WORLD_WIDTH-1, WORLD_HEIGHT-1 );

const Rect BOUNDS_RECT( -WORLD_WIDTH/4, -WORLD_HEIGHT,
			WORLD_WIDTH*5/4, WORLD_HEIGHT );
int SCREEN_WIDTH = WORLD_WIDTH;
int SCREEN_HEIGHT = WORLD_HEIGHT;

std::string Config::findFile( const std::string& name )
{
    std::string local_name("data/" + name);
    if (OS->exists(local_name)) {
        return local_name;
    }

    std::string global_name(DEFAULT_RESOURCE_PATH "/" + name);
    if (OS->exists(global_name)) {
        return global_name;
    }

    return name;
}
