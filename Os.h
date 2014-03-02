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
#ifndef OS_H
#define OS_H

#include "Event.h"
#include <stdlib.h>
#include <string>


class Accelerometer;
class WidgetParent;

class Os
{
 public:  
  virtual ~Os() {}
  virtual void  poll() {};
  virtual bool nextEvent(ToolkitEvent &ev) = 0;
  virtual long ticks() = 0;
  virtual void delay(int ms) = 0;
  virtual void init() = 0;
  virtual char* getLaunchFile() { return NULL; }
  virtual bool  openBrowser( const char* url ) = 0;
  virtual char* saveDialog( const char* path ) { return NULL; }
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

extern int npmain(int argc, char** argv);

#endif //OS_H
