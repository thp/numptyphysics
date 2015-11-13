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

#include "SDLSTBRenderer.h"

#include "Os.h"
#include "Config.h"

#include "stb_loader.h"


class EmscriptenFontData : public NP::FontData {
public:
    EmscriptenFontData(const char *filename, int size);
    ~EmscriptenFontData();

    std::string filename;
};

EmscriptenFontData::EmscriptenFontData(const char *filename, int size)
    : NP::FontData(size)
    , filename(filename)
{
}

EmscriptenFontData::~EmscriptenFontData()
{
}


SDLSTBRenderer::SDLSTBRenderer(Vec2 world_size, Vec2 framebuffer_size)
    : GLRenderer(world_size)
    , m_surface(nullptr)
    , m_texture_cache()
{
    m_surface = SDL_SetVideoMode(framebuffer_size.x, framebuffer_size.y, 0, SDL_OPENGL | SDL_RESIZABLE);

    // Query real window size (for fullscreen windows)
    framebuffer_size.x = m_surface->w;
    framebuffer_size.y = m_surface->h;
    GLRenderer::init(framebuffer_size);
}

SDLSTBRenderer::~SDLSTBRenderer()
{
}

NP::Texture
SDLSTBRenderer::load(const char *filename, bool cache)
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

    Blob *blob = Config::readBlob(filename);
    StbLoader_RGBA *rgba = StbLoader::decode_image(blob->data, blob->len);
    delete blob;
    NP::Texture result = GLRenderer::load((unsigned char *)rgba->data, rgba->w, rgba->h);
    delete rgba;

    if (cache) {
        // Store loaded image in cache
        m_texture_cache[fn] = result;
    }

    return result;
}

NP::Font
SDLSTBRenderer::load(const char *filename, int size)
{
    return NP::Font(new EmscriptenFontData(filename, size));
}

void
SDLSTBRenderer::metrics(const NP::Font &font, const char *text, int *width, int *height)
{
    EmscriptenFontData *data = static_cast<EmscriptenFontData *>(font.get());

    Blob *blob = Config::readBlob(data->filename);
    StbLoader_RGBA *rgba = StbLoader::render_font(blob->data, blob->len,
            StbLoader_Color(0.f, 0.f, 0.f, 1.f), data->size, text);
    *width = rgba->w;
    *height = rgba->h;
    delete rgba;
    delete blob;
}

NP::Texture
SDLSTBRenderer::text(const NP::Font &font, const char *text, int rgb)
{
    if (strlen(text) == 0) {
        return NP::Texture(new GLTextureData(nullptr, 10, 10));
    }

    EmscriptenFontData *data = static_cast<EmscriptenFontData *>(font.get());

    float r = 1.f * (Uint8)((rgb >> 16) & 0xff) / 255.f;
    float g = 1.f * (Uint8)((rgb >> 8) & 0xff) / 255.f;
    float b = 1.f * (Uint8)((rgb) & 0xff) / 255.f;

    Blob *blob = Config::readBlob(data->filename);
    StbLoader_RGBA *rgba = StbLoader::render_font(blob->data, blob->len,
            StbLoader_Color(r, g, b, 1.f), data->size, text);
    NP::Texture result = GLRenderer::load((unsigned char *)rgba->data, rgba->w, rgba->h);
    delete rgba;
    delete blob;
    return result;
}

void
SDLSTBRenderer::swap()
{
    SDL_GL_SwapBuffers();
}
