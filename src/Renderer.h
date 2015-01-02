/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2014 Thomas Perl <m@thp.io>
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

#ifndef NUMPTYPHYSICS_RENDERER_H
#define NUMPTYPHYSICS_RENDERER_H

#include <memory>

#include "Common.h"
#include "Path.h"

namespace NP {

class TextureData {
public:
    TextureData(int w, int h) : w(w), h(h) {}
    virtual ~TextureData() {}

    int w;
    int h;
};

typedef std::shared_ptr<TextureData> Texture;

class FontData {
public:
    FontData(int size) : size(size) {}
    virtual ~FontData() {}

    int size;
};

typedef std::shared_ptr<FontData> Font;

class FramebufferData {
public:
    FramebufferData(int w, int h) : w(w), h(h) {}
    virtual ~FramebufferData() {}

    int w;
    int h;
};

typedef std::shared_ptr<FramebufferData> Framebuffer;

class Renderer {
public:
    virtual void init() = 0;

    virtual void size(int *width, int *height) = 0;

    virtual Texture load(const char *filename, bool cache) = 0;

    virtual Framebuffer framebuffer(int width, int height) = 0;
    virtual void begin(Framebuffer &rendertarget) = 0;
    virtual void end(Framebuffer &rendertarget) = 0;
    virtual Texture retrieve(Framebuffer &rendertarget) = 0;

    virtual Rect clip(Rect rect) = 0;

    virtual void image(const Texture &texture, int x, int y, int w, int h) = 0;
    virtual void subimage(const Texture &texture, const Rect &src, const Rect &dst) = 0;
    virtual void blur(const Texture &texture, const Rect &src, const Rect &dst, float rx, float ry) = 0;
    virtual void rectangle(const Rect &rect, int rgba, bool fill) = 0;
    virtual void path(const Path &path, int rgba) = 0;

    virtual Font load(const char *filename, int size) = 0;

    virtual void metrics(const Font &font, const char *text, int *width, int *height) = 0;
    virtual Texture text(const Font &font, const char *text, int rgb) = 0;

    virtual void clear() = 0;
    virtual void flush() = 0;
    virtual void swap() = 0;
};

};

#endif /* NUMPTYPHYSICS_RENDERER_H */
