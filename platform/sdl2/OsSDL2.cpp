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
#include "GLRenderer.h"

#include <SDL.h>
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

class SDLRenderer : public GLRenderer {
public:
    SDLRenderer(int w, int h);
    ~SDLRenderer();

    virtual void init();

    virtual NP::Texture load(const char *filename);

    virtual NP::Font load(const char *filename, int size);

    virtual void metrics(const NP::Font &font, const char *text, int *width, int *height);
    virtual NP::Texture text(const NP::Font &font, const char *text, int rgb);

    virtual void swap();

private:
    SDL_Window *m_window;
    SDL_GLContext m_gl_context;
};

SDLRenderer::SDLRenderer(int w, int h)
    : GLRenderer(w, h)
    , m_window(NULL)
    , m_gl_context()
{
    m_window = SDL_CreateWindow("NumptyPhysics", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    m_gl_context = SDL_GL_CreateContext(m_window);
}

SDLRenderer::~SDLRenderer()
{
    SDL_GL_DeleteContext(m_gl_context);
    SDL_DestroyWindow(m_window);
}

void
SDLRenderer::init()
{
    GLRenderer::init();
    TTF_Init();
}

NP::Texture
SDLRenderer::load(const char *filename)
{
    std::string f = Config::findFile(filename);

    SDL_Surface *img = IMG_Load(f.c_str());
    SDL_Surface *tmp = SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_FreeSurface(img);

    NP::Texture result = GLRenderer::load((unsigned char *)tmp->pixels, img->w, img->h);
    SDL_FreeSurface(tmp);

    return result;
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
SDLRenderer::swap()
{
    SDL_GL_SwapWindow(m_window);
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
          m_renderer->init();
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
