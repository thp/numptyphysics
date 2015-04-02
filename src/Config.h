/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2008, 2009, 2010 Tim Edmonds <numptyphysics@gmail.com>
 * Coyright (c) 2012, 2014, 2015 Thomas Perl <m@thp.io>
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

#ifndef CONFIG_H
#define CONFIG_H

#include "Common.h"
#include "Os.h"

constexpr const char *PROJECT_HOMEPAGE = "http://thp.io/2015/numptyphysics/";
constexpr const char *NPSVG_NAMESPACE = "http://numptyphysics.garage.maemo.org/";

constexpr const int WORLD_WIDTH = 800;
constexpr const int WORLD_HEIGHT = 480;

constexpr const float PIXELS_PER_METREf = 10.f;
constexpr const float GRAVITY_ACCELf = 9.8f /* m/(s^2) */;
constexpr const float GRAVITY_FUDGEf = 5.0f;
constexpr const float CLOSED_SHAPE_THREHOLDf = 0.4f;
constexpr const float SIMPLIFY_THRESHOLDf = 1.0f /* pixels */;
constexpr const int MULTI_VERTEX_LIMIT = 64;

constexpr const int ITERATION_RATE = 60 /* fps */;
constexpr const int SOLVER_ITERATIONS = 8;

constexpr const int MIN_RENDER_RATE = 10 /* fps */;
constexpr const int MAX_RENDER_RATE = ITERATION_RATE /* fps */;
constexpr const int AVG_RENDER_RATE = (MIN_RENDER_RATE + MAX_RENDER_RATE) / 2;
constexpr const int HIDE_STEPS = AVG_RENDER_RATE * 4;

constexpr const float JOINT_TOLERANCE = 4.0f /* pixels */;
constexpr const float SELECT_TOLERANCE = 5.0f /* pixels */;
constexpr const float CLICK_TOLERANCE = 4.0f /* pixels */;

constexpr const float ITERATION_TIMESTEPf = 1.0f / float(ITERATION_RATE);

constexpr const float ICON_SCALE_FACTOR = 6.0f;

constexpr const int BUTTON_WIDTH = 140;
constexpr const int BUTTON_HEIGHT = 60;
constexpr const int BUTTON_SPACING = 8;

constexpr const int TAB_WIDTH = 160;
constexpr const int TAB_HEIGHT = 40;

constexpr const int DIALOG_TITLE_HEIGHT = 48;
constexpr const int MENU_COLUMNS = 1;

constexpr const int REWIND_ANIMATION_TICKS = 40;
constexpr const int REWIND_JUMP_LENGTH = 100;

constexpr const float ROPE_SEGMENT_LENGTHf = 15.f;

extern const Rect BOUNDS_RECT;


class Blob {
public:
    Blob(char *data, size_t len) : data(data), len(len) {}
    ~Blob() { free(data); }

    char *data;
    size_t len;
};

class Config {
public:
    static std::string defaultLevelPath();

    static std::string userLevelFileName(const std::string &name);
    static std::string userRecordingCollectionDir(const std::string &name);

    static std::string joinPath(const std::string &dir, const std::string &name);
    static std::string baseName(const std::string &name);

    static std::string findFile(const std::string &name);
    static std::string readFile(const std::string &name);
    static Blob *readBlob(const std::string &name);
};

#endif //CONFIG_H
