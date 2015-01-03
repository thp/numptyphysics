/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2008, 2009, 2010 Tim Edmonds <numptyphysics@gmail.com>
 * Coyright (c) 2008, 2009, 2012, 2014, 2015 Thomas Perl <m@thp.io>
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

#include <string>
#include "Common.h"
#include "Config.h"
#include "Canvas.h"
#include "Path.h"
#include "Renderer.h"


static NP::Renderer *RENDERER() { return OS->renderer(); }


Canvas::Canvas( int w, int h )
  : m_width(w)
  , m_height(h)
{
}


Canvas::~Canvas()
{
}

int Canvas::width() const
{
    return m_width;
}

int Canvas::height() const
{
    return m_height;
}

int Canvas::makeColour( int r, int g, int b ) const
{
    return (r & 0xff) << 16 | (g & 0xff) << 8 | (b & 0xff);
}

int Canvas::makeColour( int c ) const
{
    return c;
}

void Canvas::clear()
{
    EVAL_LOCAL(RENDERER);
    RENDERER->clear();
}

void Canvas::drawImage(Image &image, int x, int y)
{
    EVAL_LOCAL(RENDERER);
    RENDERER->image(image.texture(), x, y, image.width(), image.height());
}

void Canvas::drawAtlas(Image &image, const Rect &src, const Rect &dst)
{
    EVAL_LOCAL(RENDERER);
    RENDERER->subimage(image.texture(), src, dst);
}

void Canvas::drawBlur(Image &image, const Rect &src, const Rect &dst, float rx, float ry)
{
    EVAL_LOCAL(RENDERER);
    RENDERER->blur(image.texture(), src, dst, rx, ry);
}

void Canvas::drawRewind(Image &image, const Rect &src, const Rect &dst, float time, float alpha)
{
    EVAL_LOCAL(RENDERER);
    RENDERER->rewind(image.texture(), src, dst, time, alpha);
}

void Canvas::drawSaturation(Image &image, const Rect &src, const Rect &dst, float alpha)
{
    EVAL_LOCAL(RENDERER);
    RENDERER->saturation(image.texture(), src, dst, alpha);
}

void Canvas::drawPath( const Path& path, int color, int a )
{
    EVAL_LOCAL(RENDERER);
    RENDERER->path(path, color | ((a & 0xff) << 24));
}

void Canvas::drawRect( int x, int y, int w, int h, int c, bool fill, int a )
{
    drawRect(Rect(x, y, x+w, y+h), c, fill, a);
}

void Canvas::drawRect( const Rect& r, int c, bool fill, int a )
{
    EVAL_LOCAL(RENDERER);
    RENDERER->rectangle(r, c | (a << 24), fill);
}

Rect
Canvas::clip(const Rect &r)
{
    EVAL_LOCAL(RENDERER);
    return RENDERER->clip(r);
}


Window::Window(int w, int h, const char *title)
    : Canvas(w, h)
    , m_offscreen_target(nullptr)
    , m_offscreen_image(nullptr)
    , m_title(title)
{
    OS->window(w, h);

    EVAL_LOCAL(RENDERER);
    RENDERER->size(&m_width, &m_height);

    m_offscreen_target = new RenderTarget(w, h);
    m_offscreen_image = new Image(m_offscreen_target->contents());
}

Window::~Window()
{
    delete m_offscreen_image;
    delete m_offscreen_target;
}

void Window::update()
{
    EVAL_LOCAL(RENDERER);
    RENDERER->flush();
    RENDERER->swap();
}

void
Window::beginOffscreen()
{
    m_offscreen_target->begin();
}

void
Window::endOffscreen()
{
    m_offscreen_target->end();
}

Image *
Window::offscreen()
{
    return m_offscreen_image;
}

RenderTarget::RenderTarget(int w, int h)
    : Canvas(w, h)
    , m_framebuffer(RENDERER()->framebuffer(w, h))
{
}

RenderTarget::~RenderTarget()
{
}

void
RenderTarget::begin()
{
    EVAL_LOCAL(RENDERER);
    RENDERER->begin(m_framebuffer);
    clear();
}

void
RenderTarget::end()
{
    EVAL_LOCAL(RENDERER);
    RENDERER->flush();
    RENDERER->end(m_framebuffer);
}

NP::Texture
RenderTarget::contents()
{
    EVAL_LOCAL(RENDERER);
    return RENDERER->retrieve(m_framebuffer);
}


Image::Image(NP::Texture texture)
    : m_texture(texture)
    , m_width(m_texture->w)
    , m_height(m_texture->h)
{
}

Image::Image(std::string filename, bool cache)
    : m_texture(RENDERER()->load(filename.c_str(), cache))
    , m_width(m_texture->w)
    , m_height(m_texture->h)
{
}

Image::Image(NP::Font font, const char *text, int rgb)
    : m_texture(RENDERER()->text(font, text, rgb))
    , m_width(m_texture->w)
    , m_height(m_texture->h)
{
}

Image::~Image()
{
}
