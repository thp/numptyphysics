/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2009, 2010 Tim Edmonds <numptyphysics@gmail.com>
 * Coyright (c) 2012, 2014, 2015 Thomas Perl <m@thp.io>
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

#include "Common.h"
#include "Config.h"
#include "Game.h"
#include "Scene.h"
#include "Levels.h"
#include "Canvas.h"
#include "Ui.h"
#include "Font.h"
#include "Dialogs.h"
#include "Event.h"

#include "thp_timestep.h"
#include "thp_format.h"
#include "petals_log.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>


class App : private Container, public MainLoop
{
  int m_width;
  int m_height;
  bool m_quit;
  Window *m_window;
  thp::Timestep m_timestep;
public:
  App(int argc, char** argv)
    : m_width(WORLD_WIDTH)
    , m_height(WORLD_HEIGHT)
    , m_quit(false)
    , m_window(NULL)
    , m_timestep(ITERATION_RATE)
  {
      OS->ensurePath(OS->userDataDir());
      OS->init();
      setEventMap(APP_MAP);

      for (int i=0; i<argc; i++) {
          if (i < argc-1 && strcmp(argv[i], "--lang") == 0) {
              LOG_DEBUG("Trying to load translation for '%s'", argv[i+1]);
              Tr::load(thp::format("i18n/%s", argv[i+1]));
          }
      }

      m_window = new Window(m_width,m_height,"Numpty Physics");
      sizeTo(Vec2(m_width,m_height));

      Levels *levels = new Levels({Config::defaultLevelPath(), OS->userDataDir()});
      levels->dump();

      add( createGameLayer( levels, m_width, m_height ), 0, 0 );
  }

  ~App()
  {
    delete m_window;
  }

  const char* name() {return "App";}

private:

  void render()
  {
      auto world = OS->renderer()->world_rect();
      m_window->clip(world);
      m_window->clear();
      draw(*m_window, world);
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
                      LOG_DEBUG("UI: %s", toString().c_str());
                      return true;
                  default:
                      break;
              }

          case ToolkitEvent::RESIZE:
              delete m_window;
              m_window = new Window(m_width, m_height, "Numpty Physics");
              break;
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
      m_timestep.update(OS->ticks(), [this] () {
          onTick(OS->ticks());

          ToolkitEvent ev;
          while (OS->nextEvent(ev)) {
          // TODO
              processEvent(ev);
          }
      });

      render();

      return !m_quit;
  }
};

MainLoop *
npmain(int argc, char **argv)
{
    if (argc == 2) {
        const std::string arg0 = argv[1];
        if (arg0 == "-v" || arg0 == "--version") {
            printf("%s %s\n", APP, VERSION);
            exit(0);
        }
    }
    OS->init(argc, argv);
    return new App(argc, argv);
}
