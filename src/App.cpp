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

#include "Common.h"
#include "Array.h"
#include "Config.h"
#include "Game.h"
#include "Scene.h"
#include "Levels.h"
#include "Canvas.h"
#include "Ui.h"
#include "Font.h"
#include "Dialogs.h"
#include "Event.h"

#include <cstdio>
#include <string>
#include <unistd.h>


class App : private Container, public MainLoop
{
  int   m_width;
  int   m_height;
  bool  m_quit;
  int   m_renderRate;
  Window            *m_window;
  int m_iterationRate;
  int m_iterateCounter;
  int m_lastTick;
public:
  App(int argc, char** argv)
    : m_width(SCREEN_WIDTH)
    , m_height(SCREEN_HEIGHT)
    , m_quit(false)
    , m_window(NULL)
    , m_iterationRate(ITERATION_RATE)
    , m_iterateCounter(0)
    , m_lastTick(OS->ticks())
  {
      OS->ensurePath(Config::userDataDir());
      OS->init();
      setEventMap(APP_MAP);

      m_window = new Window(m_width,m_height,"Numpty Physics","NPhysics");
      sizeTo(Vec2(m_width,m_height));

      Levels* levels = new Levels();
      levels->addPath( "data" );
      levels->addPath( DEFAULT_LEVEL_PATH );
      levels->addPath( Config::userDataDir().c_str() );

      add( createGameLayer( levels, m_width, m_height ), 0, 0 );
      m_renderRate = (MIN_RENDER_RATE+MAX_RENDER_RATE)/2;
  }

  ~App()
  {
    delete m_window;
  }

  const char* name() {return "App";}

private:

#if 0
  void renderThumbnail( const char* file, int width, int height )
  {
    configureScreenTransform( width, height );
    Scene scene( true );
    if ( scene.load( file ) ) {
      printf("generating bmp %s\n", file);
      Canvas temp( width, height );
      scene.draw( temp, FULLSCREEN_RECT );
      std::string bmp( file );
      bmp += ".bmp";
      temp.writeBMP( bmp.c_str() );
    }
  }
#endif

  void render()
  {
      m_window->clear();

      draw(*m_window, FULLSCREEN_RECT);

#if DO_DRAW_FPS
          m_window->drawRect(Rect(0, 0, 50, 50), m_window->makeColour(0xbfbf8f), true);
          char buf[32];
          sprintf(buf,"%d",m_renderRate);
          Font::headingFont()->drawLeft(m_window, Vec2(20,20), buf, 0);
#endif

      m_window->update();
  }


  bool processEvent(ToolkitEvent &ev)
  {
      switch (ev.type) {
          case ToolkitEvent::QUIT:
              m_quit = true;
              return true;
          case ToolkitEvent::KEYDOWN:
              switch (ev.key) {
                  case 'q':
                      m_quit = true;
                      return true;
                  case '3':
                      fprintf(stderr,"UI: %s\n",toString().c_str());
                      return true;
                  default:
                      break;
              }
          default:
              /* do nothing */
              break;
      }

      return Container::processEvent(ev);
  }

  virtual bool onEvent( Event& ev )
  {
    switch (ev.code) {
    case Event::QUIT:
      m_quit = true;
      return true;
    default:
      break;
    }
    return false;
  }

  virtual bool step()
  {
      OS->poll();

      //assumes RENDER_RATE <= ITERATION_RATE
      while ( m_iterateCounter < m_iterationRate ) {

          onTick( m_lastTick );

          ToolkitEvent ev;
          while (OS->nextEvent(ev)) {
              processEvent(ev);
          }

          if ( m_quit ) return false;
          m_iterateCounter += m_renderRate;
      }
      m_iterateCounter -= m_iterationRate;

      render();

      int sleepMs = m_lastTick + 1000/m_renderRate - OS->ticks();

      if ( sleepMs > 1 && m_renderRate < MAX_RENDER_RATE ) {
          m_renderRate++;
          printf("increasing render rate to %dfps\n",m_renderRate);
          sleepMs = m_lastTick + 1000/m_renderRate - OS->ticks();
      }

      if ( sleepMs > 0 ) {
          OS->delay(sleepMs);
      } else {
          printf("overrun %dms\n",-sleepMs);
          if ( m_renderRate > MIN_RENDER_RATE ) {
              m_renderRate--;
              printf("decreasing render rate to %dfps\n",m_renderRate);
          } else if ( m_iterationRate > 30 ) {
              //slow down simulation time to maintain fps??
          }
      }
      m_lastTick = OS->ticks();

      return !m_quit;
  }
};


MainLoop *
npmain(int argc, char **argv)
{
    return new App(argc, argv);
}
