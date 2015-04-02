/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
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


#include "App.h"
#include "Os.h"

#include "OsSDLSTB.h"

#include "emscripten.h"


class OsEmscripten : public OsSDLSTB {
public:
    OsEmscripten()
        : OsSDLSTB()
    {
    }

    virtual bool openBrowser(const char *url)
    {
        return false;
    }

    virtual std::string userDataDir()
    {
        return "/no/user/data/dir/on/emscripten/";
    }
};

static Os *g_os = nullptr;
static MainLoop *g_mainloop = nullptr;

static void step()
{
    g_mainloop->step();
}

int main(int argc, char** argv)
{
    g_os = new OsEmscripten();
    g_mainloop = npmain(argc, argv);

    emscripten_set_main_loop(step, 60, 1);

    return 0;
}
