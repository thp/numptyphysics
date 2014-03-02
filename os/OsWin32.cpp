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

#ifdef WIN32

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL_syswm.h>
#include <windows.h>


class OsWin32 : public Os
{
  HICON icon;
  HWND hwnd;

 public:

  OsWin32()
  {
    HINSTANCE handle = GetModuleHandle(NULL);
    icon = LoadIcon(handle, MAKEINTRESOURCE(1));
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if(SDL_GetWMInfo(&wminfo) != 1) {
      //error wrong SDL version
    }
    hwnd = wminfo.window;
    SetClassLong(hwnd, GCL_HICON, (LONG)icon);
  }

  ~OsWin32()
  {
    DestroyIcon(icon);
  }

  virtual bool openBrowser( const char* url )
  {
    //??
    return false;
  }
};



Os* Os::get()
{
  static OsWin32 os;
  return &os;
}

const char Os::pathSep = '\\';



/**
 * For Windows, we have to define "WinMain" instead of the
 * usual "main" in order to get a graphical application without
 * the console window that normally pops up otherwise
 **/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		   LPSTR lpCmdLine, int nCmdShow)
{
  //TODO - pass level args here...
  npmain(0,0);
}

#endif
