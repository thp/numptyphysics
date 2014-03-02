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

//#define FORCE_16BPP

static SDL_Renderer *g_renderer = NULL;

// extract RGB colour components as 8bit values from RGB888
#define R32(p) (((p)>>16)&0xff)
#define G32(p) (((p)>>8)&0xff)
#define B32(p) ((p)&0xff)

// extract RGB colour components as 8bit values from RGB565
#define R16(p) (((p)>>8)&0xf8)
#define G16(p) (((p)>>3)&0xfc)
#define B16(p) (((p)<<3)&0xf8)

#define R16G16B16_TO_RGB888(r,g,b) \
  ((((r)<<8)&0xff0000) | ((g)&0x00ff00) | (((b)>>8)))

#define R16G16B16_TO_RGB565(r,g,b) \
  ((Uint16)( (((r)&0xf800) | (((g)>>5)&0x07e0) | (((b)>>11))&0x001f) ))

#define RGB888_TO_RGB565(p) \
  ((Uint16)( (((p)>>8)&0xf800) | (((p)>>5)&0x07e0) | (((p)>>3)&0x001f) ))


void ExtractRgb( uint32 c, int& r, int &g, int &b ) 
{
  r = R32(c); g = G32(c); b = B32(c);
}

void ExtractRgb( uint16 c, int& r, int &g, int &b )
{
  r = R16(c); g = G16(c); b = B16(c);
}


template <typename PIX> 
inline void AlphaBlend( PIX& p, int cr, int cg, int cb, int a, int ia )
{
  throw "not implemented";
}

inline void AlphaBlend( Uint16& p, int cr, int cg, int cb, int a, int ia )
{ //565
  p = R16G16B16_TO_RGB565( a * cr + ia * R16(p),
			   a * cg + ia * G16(p),
			   a * cb + ia * B16(p) );
}

inline void AlphaBlend( Uint32& p, int cr, int cg, int cb, int a, int ia )

{ //888
  p = R16G16B16_TO_RGB888( a * cr + ia * R32(p),
			   a * cg + ia * G32(p),
			   a * cb + ia * B32(p) );
}

#define ALPHA_MAX 0xff

template <typename PIX, unsigned W>
struct AlphaBrush
{
  int m_r, m_g, m_b, m_c;
  inline AlphaBrush( PIX c )
  {
    m_c = c;
    ExtractRgb( c, m_r, m_g, m_b );
  }
  inline void ink( PIX* pix, int step, int a ) 
  {
    int ia = ALPHA_MAX - a;
    int o=-W/2;
    AlphaBlend( *(pix+o*step), m_r, m_g, m_b, a, ia );
    o++;
    for ( ; o<=W/2; o++ ) {
      *(pix+o*step) = m_c;
    } 
    AlphaBlend( *(pix+o*step), m_r, m_g, m_b, ia, a );
  }
};

template <typename PIX>
struct AlphaBrush<PIX,1>
{
  int m_r, m_g, m_b, m_c;
  inline AlphaBrush( PIX c )
  {
    m_c = c;
    ExtractRgb( c, m_r, m_g, m_b );
  }
  inline void ink( PIX* pix, int step, int a ) 
  {
    int ia = ALPHA_MAX - a;
    AlphaBlend( *(pix-step), m_r, m_g, m_b, a, ia );
    AlphaBlend( *(pix), m_r, m_g, m_b, ia, a );
  }
};

template <typename PIX>
struct AlphaBrush<PIX,3>
{
  int m_r, m_g, m_b, m_c;
  inline AlphaBrush( PIX c )
  {
    m_c = c;
    ExtractRgb( c, m_r, m_g, m_b );
  }
  inline void ink( PIX* pix, int step, int a ) 
  {
    int ia = ALPHA_MAX - a;
    AlphaBlend( *(pix-step), m_r, m_g, m_b, a, ia );
    *(pix) = m_c;
    AlphaBlend( *(pix+step), m_r, m_g, m_b, ia, a );
  }
};



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
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 128);
    SDL_RenderClear(m_renderer);
    if (m_bgImage) {
        SDL_RenderCopy(m_renderer, m_bgImage->m_texture, NULL, NULL);
    }
}

void Canvas::fade( const Rect& rr ) 
{
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 128);
    int w = rr.br.x - rr.tl.x;
    int h = rr.br.y - rr.tl.y;
    SDL_Rect r = { rr.tl.x, rr.tl.y, w, h };
    SDL_RenderFillRect(m_renderer, &r);
}

Canvas* Canvas::scale( int factor ) const
{
  Canvas *c = new Canvas( width()/factor, height()/factor );  
  if ( c ) {
    if ( factor==4 && SURFACE(this)->format->BytesPerPixel==2 ) {
      const uint16 MASK2LSB = 0xe79c;
      int dpitch = SURFACE(c)->pitch / sizeof(uint16_t);
      int spitch = SURFACE(this)->pitch / sizeof(uint16_t);
      uint16_t *drow = (uint16_t*)SURFACE(c)->pixels;
      for ( int y=0;y<c->height();y++ ) {
	for ( int x=0;x<c->width();x++ ) {
          uint16 p = 0;
	  uint16_t *srow = (uint16_t*)SURFACE(this)->pixels
  	                    + (y*spitch+x)*factor;
	  for ( int yy=0;yy<4;yy++ ) {
            uint16 q = 0;
	    for ( int xx=0;xx<4;xx++ ) {
	      q += (srow[xx]&MASK2LSB)>>2;
	    }
            p += (q&MASK2LSB)>>2;
            srow += spitch;
	  }
          drow[x] = p;
	}
	drow += dpitch;
      }
    } else if (SURFACE(this)->format->BytesPerPixel==2 ) {
      int dpitch = SURFACE(c)->pitch / sizeof(uint16_t);
      int spitch = SURFACE(this)->pitch / sizeof(uint16_t);
      uint16_t *drow = (uint16_t*)SURFACE(c)->pixels;
      for ( int y=0;y<c->height();y++ ) {
	for ( int x=0;x<c->width();x++ ) {
	  uint16_t *srow = (uint16_t*)SURFACE(this)->pixels
  	                    + (y*spitch+x)*factor;
	  uint32_t r=0,g=0,b=0;
	  for ( int yy=0;yy<factor;yy++ ) {
	    for ( int xx=0;xx<factor;xx++ ) {
	      r += srow[xx] & 0xF800;
	      g += srow[xx] & 0x07e0;
	      b += srow[xx] & 0x001F;
	    }
            srow += spitch;
	  }
	  r /= factor*factor;
	  g /= factor*factor;
	  b /= factor*factor;
          drow[x] = (r&0xF800)|(g&0x07e0)|(b&0x001F);
	}
	drow += dpitch;
      }
    } else {
      for ( int y=0;y<c->height();y++ ) {
	for ( int x=0;x<c->width();x++ ) {
	  int r=0,g=0,b=0;
	  Uint8 rr,gg,bb;
	  for ( int yy=0;yy<factor;yy++ ) {
	    for ( int xx=0;xx<factor;xx++ ) {
	      SDL_GetRGB( readPixel( x*factor+xx, y*factor+yy ),
			  SURFACE(this)->format, &rr,&gg,&bb );
	      r += rr;
	      g += gg;
	      b += bb;
	    }
	  }
	  int div = factor*factor;
	  c->drawPixel( x, y, makeColour(r/div,g/div,b/div) );
	}
      }
    }
  }
  return c;
}

void Canvas::clear( const Rect& r )
{
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

void Canvas::drawPixel( int x, int y, int c )
{
  Uint32 bpp, ofs;

  bpp = SURFACE(this)->format->BytesPerPixel;
  ofs = SURFACE(this)->pitch*y;
  char* row = (char*)SURFACE(this)->pixels + ofs;

  SDL_LockSurface(SURFACE(this));
  switch ( bpp ) {
  case 2: ((Uint16*)row)[x] = c; break;
  case 4: ((Uint32*)row)[x] = c; break;
  }
  SDL_UnlockSurface(SURFACE(this));
}

int Canvas::readPixel( int x, int y ) const
{
  Uint32 bpp, ofs;
  int c;

  bpp = SURFACE(this)->format->BytesPerPixel;
  ofs = SURFACE(this)->pitch*y;
  char* row = (char*)SURFACE(this)->pixels + ofs;

  SDL_LockSurface(SURFACE(this));
  switch ( bpp ) {
  case 2: c = ((Uint16*)row)[x]; break;
  case 4: c = ((Uint32*)row)[x]; break;
  default: c=0; break;
  }
  SDL_UnlockSurface(SURFACE(this));
  return c;
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

void Canvas::drawRect( int x, int y, int w, int h, int c, bool fill )
{
    int r = (c & 0xff0000) >> 16;
    int g = (c & 0x00ff00) >> 8;
    int b = (c & 0x0000ff);
    SDL_Rect rect = { x, y, w, h };

    SDL_SetRenderDrawColor(m_renderer, r, g, b, 255);
    if (fill) {
        SDL_RenderDrawRect(m_renderer, &rect);
    } else {
        SDL_RenderFillRect(m_renderer, &rect);
    }
}

void Canvas::drawRect( const Rect& r, int c, bool fill )
{
    drawRect( r.tl.x, r.tl.y, r.br.x-r.tl.x+1, r.br.y-r.tl.y+1, c, fill );
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

  // TODO
  m_state = NULL;

  //if ( SURFACE(this) == NULL ) {
  //  throw "Unable to set video mode";
  //}

  if ( title ) {
    //SDL_WM_SetCaption( title, title );
  }
}


void Window::update( const Rect& r )
{
    SDL_RenderPresent(m_renderer);
    clear();
}

void Window::raise()
{
    // TODO
}


void Window::setSubName( const char *sub )
{
#ifdef USE_HILDON
  SDL_SysWMinfo sys;
  SDL_VERSION( &sys.version );
  SDL_GetWMInfo( &sys );

  char title[128];
  snprintf(title,128,"%s - %s\n",m_title.c_str(),sub);
  title[127] = '\0';

  // SDL_WM_SetCaption is broken on maemo4
  XStoreName( sys.info.x11.display,
	      sys.info.x11.wmwindow,
	      title );
  XStoreName( sys.info.x11.display,
	      sys.info.x11.fswindow,
	      title );
#endif
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
	int p = readPixel( x, y );
	if ( bpp==2 ) {
	  p = R16G16B16_TO_RGB888( R16(p), G16(p), B16(p) );
	}
	fwrite( &p, 3, 1, f );
      }
    }
    fclose(f);
    return 1;
  }
  return 0;
}
