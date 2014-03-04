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

#ifndef CONFIG_H
#define CONFIG_H

#include <sstream>
#include "Common.h"
#include "Os.h"

#define PROJECT_HOMEPAGE "http://github.com/harmattan/numptyphysics"

#define WORLD_WIDTH  800
#define WORLD_HEIGHT 480
#define PIXELS_PER_METREf 10.0f
#define GRAVITY_ACCELf 9.8f // m/s/s
#define GRAVITY_FUDGEf 5.0f
#define CLOSED_SHAPE_THREHOLDf 0.4f
#define SIMPLIFY_THRESHOLDf 1.0f //PIXELs //(1.0/PIXELS_PER_METREf)
#define MULTI_VERTEX_LIMIT 64

#define ITERATION_RATE    60 //fps
#define SOLVER_ITERATIONS 8

#define MIN_RENDER_RATE   10 //fps
#define MAX_RENDER_RATE   ITERATION_RATE //fps
#define AVG_RENDER_RATE   ((MIN_RENDER_RATE+MAX_RENDER_RATE)/2)
#define JOINT_TOLERANCE   4.0f //PIXELs
#define SELECT_TOLERANCE  5.0f //PIXELS_PER_METREf)
#define CLICK_TOLERANCE   4 //PIXELs

#define ITERATION_TIMESTEPf  (1.0f / (float)ITERATION_RATE)

#define HIDE_STEPS (AVG_RENDER_RATE*4)

#ifndef INSTALL_BASE_PATH
#  define INSTALL_BASE_PATH "/usr/share/numptyphysics"
#endif
#define DEFAULT_LEVEL_PATH INSTALL_BASE_PATH
#define DEFAULT_RESOURCE_PATH DEFAULT_LEVEL_PATH

#ifndef USER_BASE_PATH
#  define USER_BASE_PATH ".numptyphysics"
#endif
#define USER_LEVEL_PATH USER_BASE_PATH

#define ICON_SCALE_FACTOR 6

#define VIDEO_FPS 20
#define VIDEO_MAX_LEN 20  //seconds



extern Rect FULLSCREEN_RECT;
extern const Rect BOUNDS_RECT;
extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

#define BUTTON_WIDTH 140
#define BUTTON_HEIGHT 60
#define BUTTON_SPACING 8
#define TAB_WIDTH 160
#define TAB_HEIGHT 40
#define DIALOG_TITLE_HEIGHT 48
#define MENU_COLUMNS 1


class Config
{
 public:
  static const std::string& userDataDir()
  {
    static const std::string d = std::string(getenv("HOME")) + Os::pathSep + USER_BASE_PATH;
    return d;
  }
  static const std::string& planetRoot()
  {
    static const std::string d("http://10.10.10.10:8080");
    return d;
  }
  static std::string findFile( const std::string& name );
  static std::string readFile(const std::string &name);
};

#endif //CONFIG_H
