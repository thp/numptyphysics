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

#include <string>
#include "Common.h"
#include "Config.h"
#include "Canvas.h"
#include "Path.h"
#include "Renderer.h"

#define RENDERER  (OS->renderer())

Canvas::Canvas( int w, int h )
  : m_bgImage(NULL)
  , m_width(w)
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

void Canvas::setBackground( Image* bg )
{
  m_bgImage = bg;
}

void Canvas::clear()
{
    RENDERER->clear();
    if (m_bgImage) {
        RENDERER->image(m_bgImage->m_texture, 0, 0);
    }
}

Canvas* Canvas::scale( int factor ) const
{
    throw "Scale not implemented";
}

void Canvas::drawImage( Canvas *canvas, int x, int y )
{
    Image *image = (Image *)canvas;
    RENDERER->image(image->m_texture, x, y);
}

void Canvas::drawPath( const Path& path, int color, bool thick )
{
    RENDERER->path(path, color | 0xff000000);
}

void Canvas::drawRect( int x, int y, int w, int h, int c, bool fill, int a )
{
    drawRect(Rect(x, y, x+w, y+h), c, fill, a);
}

void Canvas::drawRect( const Rect& r, int c, bool fill, int a )
{
    RENDERER->rectangle(r, c | (a << 24), fill);
}


Window::Window( int w, int h, const char* title, const char* winclass, bool fullscreen )
    : Canvas(w, h)
    , m_title(title)
{
    OS->window(w, h);
    RENDERER->size(&m_width, &m_height);
}


void Window::update()
{
    RENDERER->flush();
    RENDERER->swap();
}

Image *
Image::fromFile(const char *filename)
{
    return new Image(RENDERER->load(filename));
}

Image *
Image::fromImage(Image *image)
{
    return new Image(image->m_texture);
}

Image *
Image::fromFont(NP::Font font, const char *text, int rgb)
{
    return new Image(RENDERER->text(font, text, rgb));
}

Image::Image(NP::Texture texture)
    : Canvas(texture->w, texture->h)
    , m_texture(texture)
{
}

Image::~Image()
{
}



int Canvas::writeBMP( const char* filename ) const
{
    throw "writing bmps is broken atm";
#if 0
#pragma pack(push,1)
  typedef struct {
    unsigned short int type;         /* Magic identifier */
    unsigned int size;               /* File size in bytes */
    unsigned short int reserved1, reserved2;
    unsigned int offset;             /* Offset to image data, bytes */
  } BMPHEADER;
  
  typedef struct {
    unsigned int size;               /* Header size in bytes      */
    int width,height;                /* Width and height of image */
    unsigned short int planes;       /* Number of colour planes   */
    unsigned short int bits;         /* Bits per pixel            */
    unsigned int compression;        /* Compression type          */
    unsigned int imagesize;          /* Image size in bytes       */
    int xresolution,yresolution;     /* Pixels per meter          */
    unsigned int ncolours;           /* Number of colours         */
    unsigned int importantcolours;   /* Important colours         */
  } BMPINFOHEADER;
  int check_BMPHEADER[(sizeof(BMPHEADER)==14)-1];
  int check_BMPINFOHEADER[(sizeof(BMPINFOHEADER)==40)-1];
#pragma pack(pop)
    
  int w = width();
  int h = height();
  BMPHEADER     head = { 'B'|('M'<<8), 14+40+w*h*3, 0, 0, 14+40 };
  BMPINFOHEADER info = { 40, w, h, 1, 24, 0, w*h*3, 100, 100, 0, 0 };

  FILE *f = fopen( filename, "wb" );
  if ( f ) {
    uint32_t bpp = 32; // FIXME
    //bpp = SURFACE(this)->format->BytesPerPixel;

    fwrite( &head, 14, 1, f );
    fwrite( &info, 40, 1, f );
    for ( int y=h-1; y>=0; y-- ) {
      for ( int x=0; x<w; x++ ) {
	int p = 0; // FIXME: read pixel at (x, y)
	fwrite( &p, 3, 1, f );
      }
    }
    fclose(f);
    return 1;
  }
  return 0;
#endif
}
