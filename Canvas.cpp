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

#include <SDL.h>
#include <SDL_image.h>

static SDL_Renderer *g_renderer = NULL;


#define SURFACE(cANVASpTR) ((SDL_Surface*)((cANVASpTR)->m_state))

Canvas::Canvas( int w, int h )
  : m_state(NULL),
    m_bgImage(NULL)
{
    m_state = SDL_CreateRGBSurface(0, w, h, 32,
            0xff0000, 0x00ff00, 0x0000ff, 0xff000000);
}


Canvas::Canvas( State state )
  : m_state(state),
    m_bgImage(NULL)
{
}

Canvas::~Canvas()
{
  if (SURFACE(this)) {
    SDL_FreeSurface(SURFACE(this));
  }
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
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
    SDL_RenderClear(m_renderer);
    if (m_bgImage) {
        SDL_RenderCopy(m_renderer, m_bgImage->m_texture, NULL, NULL);
    }
}

void Canvas::fade( const Rect& rr ) 
{
    drawRect(rr, 0x000000, true, 128);
}

Canvas* Canvas::scale( int factor ) const
{
    throw "Scale not implemented";
}

void Canvas::drawImage( Canvas *canvas, int x, int y )
{
    int w = canvas->width();
    int h = canvas->height();

    SDL_Rect sdlsrc = { 0, 0, w, h };
    SDL_Rect sdldst = { x, y, w, h };

    if (SDL_RenderCopy(m_renderer, ((Image *)canvas)->m_texture, &sdlsrc, &sdldst) != 0) {
        printf("Could not render: %s\n", SDL_GetError());
    }
}

void Canvas::drawPath( const Path& path, int color, bool thick )
{
    int r = (color & 0xff0000) >> 16;
    int g = (color & 0x00ff00) >> 8;
    int b = (color & 0x0000ff);
    SDL_SetRenderDrawColor(m_renderer, r, g, b, 255);

    // This assumes that Vec2 == int[2] and items in path are tightly packed
    SDL_RenderDrawLines(m_renderer, (SDL_Point *)&path[0], path.numPoints());
}

void Canvas::drawRect( int x, int y, int w, int h, int c, bool fill, int a )
{
    int r = (c & 0xff0000) >> 16;
    int g = (c & 0x00ff00) >> 8;
    int b = (c & 0x0000ff);
    SDL_Rect rect = { x, y, w, h };

    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
    if (fill) {
        SDL_RenderFillRect(m_renderer, &rect);
    } else {
        SDL_RenderDrawRect(m_renderer, &rect);
    }
}

void Canvas::drawRect( const Rect& r, int c, bool fill, int a )
{
    drawRect( r.tl.x, r.tl.y, r.br.x-r.tl.x+1, r.br.y-r.tl.y+1, c, fill, a );
}



Window::Window( int w, int h, const char* title, const char* winclass, bool fullscreen )
  : m_title(title)
{
  if ( winclass ) {
    char s[80];
    snprintf(s,80,"SDL_VIDEO_X11_WMCLASS=%s",winclass);
    putenv(s);
  }

  if (SDL_CreateWindowAndRenderer(w, h, SDL_WINDOW_SHOWN, &m_window, &m_renderer) != 0) {
      printf("Could not create window and renderer: %s\n", SDL_GetError());
      exit(1);
  }

  g_renderer = m_renderer;
  SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
  SDL_GetWindowSize(m_window, &m_width, &m_height);

  m_state = NULL;
}


void Window::update()
{
    SDL_RenderPresent(m_renderer);
}


Image::Image( const char* file, bool alpha )
{
  std::string f( "data/" );
  SDL_Surface* img = IMG_Load((f+file).c_str());
  if ( !img ) {
    f = std::string( DEFAULT_RESOURCE_PATH "/" );
    img = IMG_Load((f+file).c_str());
  }

  if ( img ) {
      m_width = img->w;
      m_height = img->h;
      printf("loaded image %s\n",(f+file).c_str());
      m_state = img;
  } else {
      fprintf(stderr,"failed to load image %s\n",(f+file).c_str());
      exit(1);
  }

  m_texture = SDL_CreateTextureFromSurface(g_renderer, (SDL_Surface *)m_state);
}

Image::Image(SDL_Surface *surface)
{
    m_width = surface->w;
    m_height = surface->h;
    m_state = surface;
    m_texture = SDL_CreateTextureFromSurface(g_renderer, (SDL_Surface *)m_state);
}

Image::Image(Canvas *c)
{
    SDL_Surface *s = (SDL_Surface*)c->m_state;
    m_state = SDL_ConvertSurface(s, s->format, s->flags);
    m_width = ((SDL_Surface *)m_state)->w;
    m_height = ((SDL_Surface *)m_state)->h;
    m_texture = SDL_CreateTextureFromSurface(g_renderer, (SDL_Surface *)m_state);
    printf("Texture: %p (state=%p, w=%d, h=%d)\n", m_texture, m_state, m_width, m_height);
}

Image::~Image()
{
    SDL_DestroyTexture(m_texture);
}



int Canvas::writeBMP( const char* filename ) const
{
    throw "writing bmps is broken atm";
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
    Uint32 bpp;
    bpp = SURFACE(this)->format->BytesPerPixel;

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
}
