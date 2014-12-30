/*
 * This file is part of NumptyPhysics
 * Copyright (C) 2008 Tim Edmonds
 * Copyright (C) 2014 Thomas Perl <m@thp.io>
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

#include "SDL2Renderer.h"

#include "Os.h"
#include "Config.h"

#if 0
#include <stdlib.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

#include <SDL_image.h>
#include <SDL_ttf.h>


class SDLFontData : public NP::FontData {
public:
    SDLFontData(const char *filename, int size);
    ~SDLFontData();

    TTF_Font *m_font;
};

SDLFontData::SDLFontData(const char *filename, int size)
    : NP::FontData(size)
    , m_font(TTF_OpenFont(filename, size))
{
}

SDLFontData::~SDLFontData()
{
    TTF_CloseFont(m_font);
}


SDL2Renderer::SDL2Renderer(int w, int h)
    : GLRenderer(w, h)
    , m_window(NULL)
    , m_gl_context()
    , m_texture_cache()
{
    m_window = SDL_CreateWindow("NumptyPhysics", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    m_gl_context = SDL_GL_CreateContext(m_window);
}

SDL2Renderer::~SDL2Renderer()
{
    SDL_GL_DeleteContext(m_gl_context);
    SDL_DestroyWindow(m_window);
}

void
SDL2Renderer::init()
{
    GLRenderer::init();
    TTF_Init();
}

NP::Texture
SDL2Renderer::load(const char *filename, bool cache)
{
    std::string fn(filename);

    if (cache) {
        // Cache lookup
        for (auto &item: m_texture_cache) {
            if (item.first == fn) {
                return item.second;
            }
        }
    }

    std::string f = Config::findFile(filename);

    SDL_Surface *img = IMG_Load(f.c_str());
    SDL_Surface *tmp = SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_FreeSurface(img);

    NP::Texture result = GLRenderer::load((unsigned char *)tmp->pixels, img->w, img->h);
    SDL_FreeSurface(tmp);

    if (cache) {
        // Store loaded image in cache
        m_texture_cache[fn] = result;
    }

    return result;
}

NP::Font
SDL2Renderer::load(const char *filename, int size)
{
    return NP::Font(new SDLFontData(filename, size));
}

void
SDL2Renderer::metrics(const NP::Font &font, const char *text, int *width, int *height)
{
    SDLFontData *data = static_cast<SDLFontData *>(font.get());

    TTF_SizeText(data->m_font, text, width, height);
}

NP::Texture
SDL2Renderer::text(const NP::Font &font, const char *text, int rgb)
{
    if (strlen(text) == 0) {
        return NP::Texture(new GLTextureData(nullptr, 10, 10));
    }

    SDLFontData *data = static_cast<SDLFontData *>(font.get());

    SDL_Color fg = {
        (Uint8)((rgb >> 16) & 0xff),
        (Uint8)((rgb >> 8) & 0xff),
        (Uint8)((rgb) & 0xff)
    };

    SDL_Surface *surface = TTF_RenderText_Blended(data->m_font, text, fg);
    NP::Texture result = GLRenderer::load((unsigned char *)surface->pixels, surface->w, surface->h);
    SDL_FreeSurface(surface);

    return result;
}

void
SDL2Renderer::swap()
{
    SDL_GL_SwapWindow(m_window);
}
