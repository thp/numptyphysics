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

#ifndef NUMPTYPHYSICS_GLRENDERER_H
#define NUMPTYPHYSICS_GLRENDERER_H

#include "Renderer.h"
#include "glaserlxx.h"

class GLTextureData : public NP::TextureData {
public:
    GLTextureData(unsigned char *pixels, int width, int height);
    GLTextureData(Glaserl::Texture texture);
    ~GLTextureData();

    Glaserl::Texture texture;
};

class GLFramebufferData : public NP::FramebufferData {
public:
    GLFramebufferData(int width, int height);
    ~GLFramebufferData();

    Glaserl::Framebuffer framebuffer;
};

class GLRendererPriv;

class GLRenderer : public NP::Renderer {
public:
    GLRenderer(Vec2 world_size);
    ~GLRenderer();

    void init(Vec2 framebuffer_size);
    void mapXY(int &x, int &y);
    void projectXY(int &x, int &y);

    virtual Vec2 framebuffer_size();
    virtual Vec2 world_size();

    virtual NP::Texture load(unsigned char *pixels, int w, int h);

    virtual NP::Framebuffer framebuffer(Vec2 size);
    virtual void begin(NP::Framebuffer &rendertarget, Rect world_rect);
    virtual void end(NP::Framebuffer &rendertarget);
    virtual NP::Texture retrieve(NP::Framebuffer &rendertarget);

    virtual Rect clip(Rect rect);

    virtual void image(const NP::Texture &texture, int x, int y, int w, int h);
    virtual void subimage(const NP::Texture &texture, const Rect &src, const Rect &dst);
    virtual void blur(const NP::Texture &texture, const Rect &src, const Rect &dst, float rx, float ry);
    virtual void rewind(const NP::Texture &texture, const Rect &src, const Rect &dst, float t, float a);
    virtual void saturation(const NP::Texture &texture, const Rect &src, const Rect &dst, float a);
    virtual void rectangle(const Rect &r, int rgba, bool fill);
    virtual void path(const Path &p, int rgba);

    virtual void clear();
    virtual void flush();

private:
    Vec2 _world_size;
    GLRendererPriv *priv;
};

#endif /* NUMPTYPHYSICS_GLRENDERER_H */
