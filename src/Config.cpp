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

#include <sstream>
#include <fstream>


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

    std::string global_name(INSTALL_BASE_PATH "/" + name);
    if (OS->exists(global_name)) {
        return global_name;
    }

    return name;
}

std::string
Config::readFile(const std::string &name)
{
    std::string filename = findFile(name);
    std::ifstream is(filename.c_str(), std::ios::in);

    is.seekg(0, is.end);
    size_t length = is.tellg();
    is.seekg(0, is.beg);

    char *buffer = new char [length+1];
    buffer[length] = '\0';

    is.read (buffer,length);
    is.close();
    std::string result = buffer;
    delete[] buffer;

    return result;
}

std::string
Config::defaultLevelPath()
{
    return INSTALL_BASE_PATH;
}

std::string
Config::userLevelFileName(const std::string &name)
{
    return OS->userDataDir() + Os::pathSep + name;
}

std::string
Config::userRecordingCollectionDir(const std::string &name)
{
    return OS->userDataDir() + Os::pathSep + "Recordings" + Os::pathSep + name;
}

std::string
Config::joinPath(const std::string &dir, const std::string &name)
{
    return dir + Os::pathSep + name;
}

std::string
Config::baseName(const std::string &name)
{
    size_t sep = name.rfind(Os::pathSep);
    return (sep == std::string::npos) ? name : name.substr(sep);
}
