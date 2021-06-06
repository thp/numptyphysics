/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2008, 2009 Tim Edmonds <numptyphysics@gmail.com>
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


#include "Os.h"

#ifndef _WIN32
#include <gio/gio.h>


class GLibString {
public:
    GLibString(gchar *value) : value(value) {}
    ~GLibString() { g_free(value); }

    operator std::string() { return value; }

private:
    gchar *value;
};

#endif


class OsFreeDesktop : public Os {
public:
    OsFreeDesktop()
        : Os()
    {
    }

#ifndef _WIN32
    virtual bool openBrowser(const char *url)
    {
        return g_app_info_launch_default_for_uri (url, NULL, NULL) == TRUE;
    }

    virtual std::string userDataDir()
    {
        return GLibString(g_build_filename(g_get_user_data_dir(), appName().c_str(), NULL));
    }
#else
    virtual bool openBrowser(const char *url)
    {
        // TODO
        return false;
    }

    virtual std::string userDataDir()
    {
        // TODO
        return ".";
    }
#endif
};
