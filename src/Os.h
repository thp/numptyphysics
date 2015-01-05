/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2008, 2009, 2010 Tim Edmonds <numptyphysics@gmail.com>
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
#ifndef OS_H
#define OS_H

#include "Event.h"
#include "Renderer.h"
#include <stdlib.h>
#include <string>


class Accelerometer;
class WidgetParent;

class MainLoop {
public:
    MainLoop() {}
    virtual ~MainLoop() {}

    virtual bool step() = 0;
};

class Os
{
 public:
  Os();
  virtual ~Os() {}
  virtual bool nextEvent(ToolkitEvent &ev) = 0;
  virtual long ticks() = 0;
  virtual void delay(int ms) = 0;
  virtual void init() = 0;
  virtual void window(Vec2 world_size) = 0;
  virtual NP::Renderer *renderer() = 0;

  void init(int argc, char **argv);
  std::string appName();
  std::string globalDataDir();

  virtual bool openBrowser(const char *url) = 0;
  virtual std::string userDataDir() = 0;

  virtual Accelerometer*  getAccelerometer() { return NULL; }
  virtual EventMap* getEventMap( EventMapType type );
  virtual void decorateGame( WidgetParent* game ) {}
  bool ensurePath(const std::string& path);
  bool exists(const std::string& file);
  static Os* get();
  static const char pathSep;
};

class OsObj
{
 public:
  Os* operator->() { return Os::get(); }
};
extern OsObj OS;

#endif //OS_H
