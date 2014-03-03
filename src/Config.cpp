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

const int brushColours[] = {
  0xb80000, //red
  0xeec900, //yellow
  0x000077, //blue
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

const int NUM_BRUSHES = (sizeof(brushColours)/sizeof(brushColours[0]));

std::string Config::findFile( const std::string& name )
{
  std::string p( "data/" );
  FILE *fd = fopen( (p+name).c_str(), "rb"  );
  if ( !fd ) {
    p = std::string( DEFAULT_RESOURCE_PATH "/" );
    fd = fopen( (p+name).c_str(), "rb" );
  }
  if ( fd ) {
    fclose(fd);
    return p+name;
  }
  return name;
}

