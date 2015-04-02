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


class OsSDL1 : public OsSDLSTB {
public:
    OsSDL1()
        : OsSDLSTB()
    {
    }

    virtual bool openBrowser(const char *url)
    {
        return false;
    }

    virtual std::string userDataDir()
    {
        return ".numptyphysics-data-sdl1";
    }
};

int main(int argc, char** argv)
{
    std::shared_ptr<Os> os(new OsSDL1());

    std::shared_ptr<MainLoop> mainloop(npmain(argc, argv));
    while (mainloop->step());

    return 0;
}
