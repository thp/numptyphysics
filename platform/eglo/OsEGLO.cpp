/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2014, 2015, 2016 Thomas Perl <m@thp.io>
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

#include "EGLOSTBRenderer.h"

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#include "eglo.h"


static void mapEGLOEventToToolkitEvent(EgloEvent &e, ToolkitEvent &ev)
{
    switch (e.type) {
        case EGLO_MOUSE_DOWN:
            ev.type = ToolkitEvent::PRESS;
            ev.x = e.mouse.x;
            ev.y = e.mouse.y;
            ev.finger = 0;
            ev.key = 1;
            break;
        case EGLO_MOUSE_UP:
            ev.type = ToolkitEvent::RELEASE;
            ev.x = e.mouse.x;
            ev.y = e.mouse.y;
            ev.finger = 0;
            ev.key = 1;
            break;
        case EGLO_MOUSE_MOTION:
            ev.type = ToolkitEvent::MOVE;
            ev.x = e.mouse.x;
            ev.y = e.mouse.y;
            ev.finger = 0;
            ev.key = 1;
            break;
       //case EGL_KEY_DOWN:
            //ev.type = ToolkitEvent::KEYDOWN;
            //ev.x = ev.y = ev.finger = 0;
            //ev.key = mapEGLOKeyToNumptyKey(e.key.key);
            //break;
        case EGLO_QUIT:
            ev.type = ToolkitEvent::QUIT;
            break;
        default:
            ev.type = ToolkitEvent::NONE;
            break;
    }
}

class OsEGLOSTB : public Os {
public:
    OsEGLOSTB()
        : Os()
        , m_renderer(nullptr)
        , m_width(0)
        , m_height(0)
    {
    }

    virtual void init()
    {
        eglo_init(&m_width, &m_height, 2);
    }

    virtual void window(Vec2 world_size)
    {
        if (!m_renderer) {
            m_renderer = new EGLOSTBRenderer(world_size, Vec2(m_width, m_height));
        }
    }

    virtual NP::Renderer *renderer()
    {
        return m_renderer;
    }

    virtual bool nextEvent(ToolkitEvent &ev)
    {
        EgloEvent e;
        while (eglo_next_event(&e)) {
            mapEGLOEventToToolkitEvent(e, ev);
            m_renderer->mapXY(ev.x, ev.y);
            return true;
        }

        return false;
    }

    virtual long ticks()
    {
        static bool time_inited = false;
        static struct timespec ts0;

        if (!time_inited) {
            time_inited = true;
            clock_gettime(CLOCK_MONOTONIC, &ts0);
        }

        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (ts.tv_sec - ts0.tv_sec) * 1000 + (ts.tv_nsec - ts0.tv_nsec) / (1000 * 1000);
    }

    virtual void delay(int ms)
    {
        ::usleep(1000 * ms);
    }

    virtual bool openBrowser(const char *url)
    {
        return false;
    }

    virtual std::string userDataDir()
    {
        return ".numptyphysics-data-eglo";
    }

private:
    EGLOSTBRenderer *m_renderer;
    int m_width;
    int m_height;
};

int main(int argc, char** argv)
{
    std::shared_ptr<Os> os(new OsEGLOSTB());

    std::shared_ptr<MainLoop> mainloop(npmain(argc, argv));
    while (mainloop->step());

    return 0;
}
