/*
 * This file is part of NumptyPhysics
 * Copyright (C) 2008 Tim Edmonds
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

#ifndef CANVAS_H
#define CANVAS_H

#include "Common.h"
#include "Renderer.h"

class Path;
class Image;

class Canvas
{
public:
  Canvas( int w, int h );
  virtual ~Canvas();
  int width() const;
  int height() const;
  int  makeColour( int c ) const;
  int  makeColour( int r, int g, int b ) const;
  void setBackground( Image* bg );
  void clear();
  void drawImage(Image &image, int x, int y);
  void drawPath( const Path& path, int color, bool thick=false );
  void drawRect( int x, int y, int w, int h, int c, bool fill=true, int a=255 );
  void drawRect( const Rect& r, int c, bool fill=true, int a=255 );
  int writeBMP( const char* filename ) const;
protected:
  Image* m_bgImage; 
  int m_width;
  int m_height;
};

class Window : public Canvas
{
 public:
  Window( int w, int h, const char* title=NULL, const char* winclass=NULL, bool fullscreen=false );
  void update();
 protected:
  std::string m_title;
};

class RenderTarget : public Canvas
{
public:
    RenderTarget(int w, int h);
    ~RenderTarget();

    void begin();
    void end();

    NP::Texture contents();

private:
    NP::Framebuffer m_framebuffer;
};


class Image
{
public:
    Image(NP::Texture texture);
    Image(std::string filename);
    Image(NP::Font font, const char *text, int rgb);
    ~Image();

    int width() const { return m_width; }
    int height() const { return m_height; }

    void scale(float scale) { m_width *= scale; m_height *= scale; }

    NP::Texture texture() const { return m_texture; }

private:
    NP::Texture m_texture;
    int m_width;
    int m_height;
};


#endif //CANVAS_H
