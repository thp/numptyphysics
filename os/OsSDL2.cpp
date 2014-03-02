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

#include "Os.h"
#include "Config.h"
#include <stdlib.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "Renderer.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

class SDLTextureData : public NP::TextureData {
public:
    SDLTextureData(SDL_Renderer *renderer, SDL_Surface *surface);
    ~SDLTextureData();

    SDL_Texture *m_texture;
};

SDLTextureData::SDLTextureData(SDL_Renderer *renderer, SDL_Surface *surface)
    : NP::TextureData(surface->w, surface->h)
    , m_texture(SDL_CreateTextureFromSurface(renderer, surface))
{
    SDL_FreeSurface(surface);
}

SDLTextureData::~SDLTextureData()
{
    SDL_DestroyTexture(m_texture);
}

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

class SDLRenderer : public NP::Renderer {
public:
    SDLRenderer(int w, int h);
    ~SDLRenderer();

    virtual void size(int *width, int *height);

    virtual NP::Texture load(const char *filename);
    virtual NP::Texture load(unsigned char *rgba, int width, int height);

    virtual void image(const NP::Texture &texture, int x, int y);
    virtual void rectangle(const Rect &r, int rgba, bool fill);
    virtual void path(const Path &p, int rgba);

    virtual NP::Font load(const char *filename, int size);

    virtual void metrics(const NP::Font &font, const char *text, int *width, int *height);
    virtual NP::Texture text(const NP::Font &font, const char *text, int rgb);

    virtual void clear();
    virtual void swap();

private:
    SDL_Window *m_window;
    SDL_Renderer *m_renderer;
};

SDLRenderer::SDLRenderer(int w, int h)
    : m_window(NULL)
    , m_renderer(NULL)
{
    if (SDL_CreateWindowAndRenderer(w, h, SDL_WINDOW_SHOWN, &m_window, &m_renderer) != 0) {
        throw std::string("Could not create window: ") + std::string(SDL_GetError());
    }

    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    TTF_Init();
}

SDLRenderer::~SDLRenderer()
{
}

void
SDLRenderer::size(int *width, int *height)
{
    SDL_GetWindowSize(m_window, width, height);
}

NP::Texture
SDLRenderer::load(const char *filename)
{
    std::string f("data/");

    SDL_Surface *img = IMG_Load((f+filename).c_str());
    if (!img) {
        f = std::string(DEFAULT_RESOURCE_PATH "/");
        img = IMG_Load((f+filename).c_str());
    }

    return NP::Texture(new SDLTextureData(m_renderer, img));
}

NP::Texture
SDLRenderer::load(unsigned char *rgba, int width, int height)
{
    SDL_Surface *img = (SDL_Surface *)rgba;
    return NP::Texture(new SDLTextureData(m_renderer, img));
}

void
SDLRenderer::image(const NP::Texture &texture, int x, int y)
{
    SDLTextureData *data = static_cast<SDLTextureData *>(texture.get());

    SDL_Rect sdlsrc = { 0, 0, data->w, data->h };
    SDL_Rect sdldst = { x, y, data->w, data->h };

    SDL_RenderCopy(m_renderer, data->m_texture, &sdlsrc, &sdldst);
}

void
SDLRenderer::rectangle(const Rect &rect, int rgba, bool fill)
{
    int a = (rgba & 0xff000000) >> 24;
    int r = (rgba & 0x00ff0000) >> 16;
    int g = (rgba & 0x0000ff00) >> 8;
    int b = (rgba & 0x000000ff);
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);

    SDL_Rect sdlrect = { rect.tl.x, rect.tl.y, rect.br.x - rect.tl.x, rect.br.y - rect.tl.y };
    if (fill) {
        SDL_RenderFillRect(m_renderer, &sdlrect);
    } else {
        SDL_RenderDrawRect(m_renderer, &sdlrect);
    }
}

void
SDLRenderer::path(const Path &path, int rgba)
{
    int a = (rgba & 0xff000000) >> 24;
    int r = (rgba & 0x00ff0000) >> 16;
    int g = (rgba & 0x0000ff00) >> 8;
    int b = (rgba & 0x000000ff);
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);

    // This assumes that Vec2 == int[2] and items in path are tightly packed
    SDL_RenderDrawLines(m_renderer, (SDL_Point *)&path[0], path.numPoints());
}

NP::Font
SDLRenderer::load(const char *filename, int size)
{
    return NP::Font(new SDLFontData(filename, size));
}

void
SDLRenderer::metrics(const NP::Font &font, const char *text, int *width, int *height)
{
    SDLFontData *data = static_cast<SDLFontData *>(font.get());

    TTF_SizeText(data->m_font, text, width, height);
}

NP::Texture
SDLRenderer::text(const NP::Font &font, const char *text, int rgb)
{
    SDLFontData *data = static_cast<SDLFontData *>(font.get());

    SDL_Color fg = {
        (rgb >> 16) & 0xff,
        (rgb >> 8) & 0xff,
        (rgb) & 0xff
    };

    SDL_Surface *surface = TTF_RenderText_Blended(data->m_font, text, fg);
    return NP::Texture(new SDLTextureData(m_renderer, surface));
}

void
SDLRenderer::clear()
{
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
    SDL_RenderClear(m_renderer);
}

void
SDLRenderer::swap()
{
    SDL_RenderPresent(m_renderer);
}


class OsFreeDesktop : public Os
{
 public:
  OsFreeDesktop()
    : m_fifo(NULL)
    , m_cmdReady(false)
    , m_cmdPos(0)
    , m_renderer(NULL)
  {
  }

  virtual void window(int w, int h)
  {
      if (!m_renderer) {
          m_renderer = new SDLRenderer(w, h);
      }
  }

  virtual NP::Renderer *renderer()
  {
      return m_renderer;
  }

  virtual bool openBrowser( const char* url )
  {
    if ( url && strlen(url) < 200 ) {
      char buf[256];
      snprintf(buf,256,"xdg-open '%s'",url);
      if ( system( buf ) == 0 ) {
	return true;
      }
    }
    return false;
  }

  virtual char* saveDialog( const char* path )
  {
    //TODO - gtk?
    return NULL;
  }

  virtual void poll() 
  {
    if ( m_fifo && !m_cmdReady ) {
      int c = 0;
      while ( c != EOF ) {
	c = fgetc( m_fifo );
	m_cmdBuffer[m_cmdPos++] = c;
	if ( c == 0 ) {
	  m_cmdReady = false;
	  break;
	}
      }
    }
  }

  virtual bool nextEvent(ToolkitEvent &ev)
  {
      SDL_Event e;
      if (!SDL_PollEvent(&e)) {
          return false;
      }

      switch (e.type) {
          case SDL_MOUSEBUTTONDOWN:
              ev.type = ToolkitEvent::PRESS;
              ev.x = e.button.x;
              ev.y = e.button.y;
              ev.finger = e.button.which;
              ev.key = e.button.button;
              break;
          case SDL_MOUSEBUTTONUP:
              ev.type = ToolkitEvent::RELEASE;
              ev.x = e.button.x;
              ev.y = e.button.y;
              ev.finger = e.button.which;
              ev.key = e.button.button;
              break;
          case SDL_MOUSEMOTION:
              ev.type = ToolkitEvent::MOVE;
              ev.x = e.motion.x;
              ev.y = e.motion.y;
              ev.finger = e.motion.which;
              ev.key = e.motion.state;
              break;
          case SDL_KEYDOWN:
              ev.type = ToolkitEvent::KEYDOWN;
              ev.x = ev.y = ev.finger = 0;
              ev.key = e.key.keysym.sym;
              break;
          case SDL_QUIT:
              ev.type = ToolkitEvent::QUIT;
              break;
          default:
              ev.type = ToolkitEvent::NONE;
              break;
      }

      //printf("Got event: %d (%d, %d - %d, %d [%c])\n", ev.type, ev.x, ev.y,
      //        ev.finger, ev.key, (ev.key > 30) ? ev.key : '?');
      return true;
  }

  virtual long ticks()
  {
      return SDL_GetTicks();
  }

  virtual void delay(int ms)
  {
      SDL_Delay(ms);
  }

  virtual void init()
  {
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0) {
      throw "Couldn't initialize SDL";
    }
  }

  virtual char *getLaunchFile() 
  {
    poll();
    if ( m_cmdReady ) {
      m_cmdPos = 0;
      m_cmdReady = false;
      return m_cmdBuffer;
    }
    return NULL;
  }

  bool setupPipe( int argc, char** argv )
  {
    return true;
    std::string fifoFile = Config::userDataDir()+Os::pathSep+".pipe";
    FILE *fifo = fopen( fifoFile.c_str(), "w" );
    if ( fifo ) {
      for ( int i=1; i<argc; i++ ) {
	fwrite( argv[i], 1, strlen(argv[i])+1, fifo );
      }
      fclose(fifo);
      return false;
    } else {
      // Create the FIFO file
      umask(0);
      if (mknod( fifoFile.c_str(), S_IFIFO|0666, 0)) {
	fprintf(stderr, "mknod() failed\n");
      } else {
	m_fifo = fopen( fifoFile.c_str(), "r");
      }
      return true;
    }
  }

private:
  FILE *m_fifo;
  char m_cmdBuffer[128];
  int  m_cmdPos;
  bool m_cmdReady;
  SDLRenderer *m_renderer;
};


Os* Os::get()
{
  static OsFreeDesktop os;
  return &os;
}

const char Os::pathSep = '/';

int main(int argc, char** argv)
{
    try {
        if ( ((OsFreeDesktop*)Os::get())->setupPipe(argc,argv) ) {
            std::shared_ptr<MainLoop> mainloop(npmain(argc, argv));
            while (mainloop->step());
        }
    } catch ( const char* e ) {
        fprintf(stderr, "*** CAUGHT: %s", e);
    }

    return 0;
}
