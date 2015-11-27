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

#include "SDLSTBRenderer.h"

#include "SDL.h"


static int mapSDLKeyToNumptyKey(int key)
{
    switch (key) {
        case SDLK_SPACE: return NUMPTYKEY_SPACE;
        case SDLK_RETURN: return NUMPTYKEY_RETURN;
        case SDLK_ESCAPE: return NUMPTYKEY_ESCAPE;
        case SDLK_BACKSPACE: return NUMPTYKEY_BACKSPACE;

        case SDLK_DOWN: return NUMPTYKEY_DOWN;
        case SDLK_UP: return NUMPTYKEY_UP;
        case SDLK_LEFT: return NUMPTYKEY_LEFT;
        case SDLK_RIGHT: return NUMPTYKEY_RIGHT;

        case SDLK_u: return NUMPTYKEY_u;
        case SDLK_s: return NUMPTYKEY_s;
        case SDLK_m: return NUMPTYKEY_m;
        case SDLK_e: return NUMPTYKEY_e;
        case SDLK_r: return NUMPTYKEY_r;
        case SDLK_n: return NUMPTYKEY_n;
        case SDLK_p: return NUMPTYKEY_p;
        case SDLK_v: return NUMPTYKEY_v;
        case SDLK_q: return NUMPTYKEY_q;

        case SDLK_F4: return NUMPTYKEY_F4;
        case SDLK_F6: return NUMPTYKEY_F6;
        case SDLK_F7: return NUMPTYKEY_F7;
        default: return NUMPTYKEY_NONE;
    }

    return NUMPTYKEY_NONE;
}

static void mapSDLEventToToolkitEvent(SDL_Event &e, ToolkitEvent &ev)
{
    switch (e.type) {
        case SDL_MOUSEBUTTONDOWN:
            ev.type = ToolkitEvent::PRESS;
            ev.x = e.button.x;
            ev.y = e.button.y;
#if defined(EMSCRIPTEN)
            ev.finger = 0;
#else
            ev.finger = e.button.which;
#endif /* defined(EMSCRIPTEN) */
            ev.key = e.button.button;
            break;
        case SDL_MOUSEBUTTONUP:
            ev.type = ToolkitEvent::RELEASE;
            ev.x = e.button.x;
            ev.y = e.button.y;
#if defined(EMSCRIPTEN)
            ev.finger = 0;
#else
            ev.finger = e.button.which;
#endif /* defined(EMSCRIPTEN) */
            ev.key = e.button.button;
            break;
        case SDL_MOUSEMOTION:
            ev.type = ToolkitEvent::MOVE;
            ev.x = e.motion.x;
            ev.y = e.motion.y;
#if defined(EMSCRIPTEN)
            ev.finger = 0;
#else
            ev.finger = e.motion.which;
#endif /* defined(EMSCRIPTEN) */
            ev.key = e.motion.state;
            break;
        case SDL_KEYDOWN:
            ev.type = ToolkitEvent::KEYDOWN;
            ev.x = ev.y = ev.finger = 0;
            ev.key = mapSDLKeyToNumptyKey(e.key.keysym.sym);
            break;
        case SDL_VIDEORESIZE:
            ev.type = ToolkitEvent::RESIZE;
            ev.x = e.resize.w;
            ev.y = e.resize.h;
            break;
        case SDL_QUIT:
            ev.type = ToolkitEvent::QUIT;
            break;
        default:
            ev.type = ToolkitEvent::NONE;
            break;
    }
}

class OsSDLSTB : public Os {
public:
    OsSDLSTB()
        : Os()
        , m_renderer(nullptr)
    {
    }

    virtual void init()
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
            exit(1);
        }
    }

    virtual void window(Vec2 world_size)
    {
        if (!m_renderer) {
            m_renderer = new SDLSTBRenderer(world_size, world_size);
        }
    }

    virtual NP::Renderer *renderer()
    {
        return m_renderer;
    }

    virtual bool nextEvent(ToolkitEvent &ev)
    {
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            mapSDLEventToToolkitEvent(e, ev);
            m_renderer->mapXY(ev.x, ev.y);
            if (ev.type == ToolkitEvent::RESIZE) {
                Vec2 world_size = m_renderer->world_size();
                delete m_renderer;
                m_renderer = new SDLSTBRenderer(world_size, Vec2(ev.x, ev.y));
            }
            return true;
        }

        return false;
    }

    virtual long ticks()
    {
        return SDL_GetTicks();
    }

    virtual void delay(int ms)
    {
        SDL_Delay(ms);
    }

private:
    SDLSTBRenderer *m_renderer;
};
