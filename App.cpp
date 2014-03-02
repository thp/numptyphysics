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
#include <sys/stat.h>
#include <unistd.h>
#include <SDL.h>


class App : private Container
{
  int   m_width;
  int   m_height;
  bool  m_rotate;  
  bool  m_thumbnailMode;
  bool  m_videoMode;
  std::string m_testOp;
  bool  m_quit;
  bool  m_drawFps;
  int   m_renderRate;
  Array<const char*> m_files;
  Window            *m_window;
public:
  App(int argc, char** argv)
    : m_width(SCREEN_WIDTH),
      m_height(SCREEN_HEIGHT),
      m_rotate(false),
      m_thumbnailMode(false),
      m_videoMode(false),
      m_quit(false),
      m_drawFps(false),
      m_window(NULL)
  {
    for ( int i=1; i<argc; i++ ) {
      if ( strcmp(argv[i],"-test")==0 && i < argc-1) {
	m_testOp = argv[i+++1];
      } else if ( strcmp(argv[i],"-bmp")==0 ) {
	m_thumbnailMode = true;
      } else if ( strcmp(argv[i],"-video")==0 ) {
	m_videoMode = true;
      } else if ( strcmp(argv[i],"-fps")==0 ) {
	m_drawFps = true;
      } else if ( strcmp(argv[i],"-rotate")==0 ) {
	m_rotate = true;
      } else if ( strcmp(argv[i],"-geometry")==0 && i<argc-1) {
	int ww, hh;
	if ( sscanf(argv[i+1],"%dx%d",&ww,&hh) ==2 ) {
	  m_width = ww; 
	  m_height = hh;
	}
	i++;
      } else {
	m_files.append( argv[i] );
      }
    }
    init();
    setEventMap(APP_MAP);
  }

  ~App()
  {
    delete m_window;
  }

  const char* name() {return "App";}

  void run()
  {
    if ( m_testOp.length() > 0 ) {
      test( m_testOp );
    } else if ( m_thumbnailMode ) {
      for ( int i=0; i<m_files.size(); i++ ) {
	renderThumbnail( m_files[i], m_width, m_height );
      }
    } else if ( m_videoMode ) {
      for ( int i=0; i<m_files.size(); i++ ) {
	renderVideo( m_files[i], m_width, m_height );
      }
    } else {      
      m_window = new Window(m_width,m_height,"Numpty Physics","NPhysics");
      sizeTo(Vec2(m_width,m_height));
      runGame( m_files, m_width, m_height );
    }
  }

private:

  void init()
  {
    if ( m_thumbnailMode || m_videoMode || m_testOp.length() > 0 ) {
      putenv((char*)"SDL_VIDEODRIVER=dummy");
    } else {
      putenv((char*)"SDL_VIDEO_X11_WMCLASS=NPhysics");
      OS->ensurePath(Config::userDataDir());
    }
    
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0) {
      throw "Couldn't initialize SDL";
    }   
  }


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


  void renderVideo( const char* file, int width, int height )
  {
    configureScreenTransform( width, height );

    Levels* levels = new Levels();
    levels->addPath( file );
    add( createGameLayer( levels, width, height ) );

    Rect area(0,0,width,height);
    Canvas canvas(width,height);
    int iterateCounter = 0;

    for ( int f=0; f<VIDEO_FPS*VIDEO_MAX_LEN; f++ ) {
      while ( iterateCounter < ITERATION_RATE ) {
	m_children[0]->onTick( f*1000/VIDEO_FPS );
	iterateCounter += VIDEO_FPS;
      }
      iterateCounter -= ITERATION_RATE;
      m_children[0]->draw( canvas, area );
      char bfile[128];
      sprintf(bfile,"%s.%04d.bmp",file,f);
      canvas.writeBMP( bfile );
    }
  }

  void runGame( Array<const char*>& files, int width, int height )
  {
    Levels* levels = new Levels();
    
    if ( files.size() > 0 ) {
      for ( int i=0; i<files.size(); i++ ) {
	levels->addPath( files[i] );
      }
    } else {
      struct stat st;
      if ( stat("Game.cpp",&st)==0 ) {
	levels->addPath( "data" );
      } else {
	levels->addPath( DEFAULT_LEVEL_PATH );
      }
      levels->addPath( Config::userDataDir().c_str() );
    }
        
    add( createGameLayer( levels, width, height ), 0, 0 );
    mainLoop();
  }

  void render()
  {
      draw(*m_window, FULLSCREEN_RECT);

      if ( m_drawFps ) {
	m_window->drawRect( Rect(0,0,50,50), m_window->makeColour(0xbfbf8f), true );
	char buf[32];
	sprintf(buf,"%d",m_renderRate);
	Font::headingFont()->drawLeft( m_window, Vec2(20,20), buf, 0 );
	m_window->update( Rect(0,0,50,50) );
      }

      m_window->update(FULLSCREEN_RECT);
  }


  void waitActive()
  {
    SDL_Event ev;
    while ( SDL_WaitEvent(&ev) ) {
      OS->poll();
      switch( ev.type ) {
      case SDL_QUIT:
	m_quit = true;
	return;
#if 0
      case SDL_ACTIVEEVENT:
	if (ev.active.gain > 0) {
	  return;
	}
	break;
#endif
      }
    }
  }

  bool processEvent( SDL_Event &ev )
  {
    switch( ev.type ) {
    case SDL_QUIT:
      m_quit = true;
      return true;
#if 0
    case SDL_ACTIVEEVENT:
      printf("active: %d\n",ev.active.gain);
      if (ev.active.gain == 0) {
	waitActive();
      }
      break;
#endif
    case SDL_MOUSEBUTTONDOWN:
      if (ev.button.x < 10) 
	printf("clicky: %d,%d\n",ev.button.x,ev.button.y);
      break;
    case SDL_KEYDOWN:
      switch ( ev.key.keysym.sym ) {
      case SDLK_1:
      case SDLK_f:
	m_drawFps = !m_drawFps;
	return true;
      case SDLK_q:
	m_quit = true;
	return true;
      case SDLK_3:
	fprintf(stderr,"UI: %s\n",toString().c_str());
	return true;
      default:
	break;
      }
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
  
  void mainLoop()
  {
    render();

    m_renderRate = (MIN_RENDER_RATE+MAX_RENDER_RATE)/2;
    int iterationRate = ITERATION_RATE;
    int iterateCounter = 0;
    int lastTick = SDL_GetTicks();
    bool isComplete = false;    

    while ( !m_quit ) {
      OS->poll();

      //assumes RENDER_RATE <= ITERATION_RATE
      while ( iterateCounter < iterationRate ) {

	onTick( lastTick );
    
	SDL_Event ev;
	while ( SDL_PollEvent(&ev) ) {
	  processEvent(ev);
	}

	if ( m_quit ) return;
	iterateCounter += m_renderRate;
      }
      iterateCounter -= iterationRate;

      render();

      int sleepMs = lastTick + 1000/m_renderRate -  SDL_GetTicks();

      if ( sleepMs > 1 && m_renderRate < MAX_RENDER_RATE ) {
	m_renderRate++;
	//printf("increasing render rate to %dfps\n",m_renderRate);
	sleepMs = lastTick + 1000/m_renderRate -  SDL_GetTicks();
      }

      if ( sleepMs > 0 ) {
	SDL_Delay( sleepMs );
      } else {
	//printf("overrun %dms\n",-sleepMs);
	if ( m_renderRate > MIN_RENDER_RATE ) {
	  m_renderRate--;
	  //printf("decreasing render rate to %dfps\n",m_renderRate);
	} else if ( iterationRate > 30 ) {
	  //slow down simulation time to maintain fps??
	}
      }
      lastTick = SDL_GetTicks();
    }
  }
  

  void test( std::string op ) 
  {
    if ( op=="levels" ) {
      Levels levels;
      for ( int i=0; i<m_files.size(); i++ ) {
	levels.addPath( m_files[i] );
	fprintf(stderr,"LEVELS after %s\n",m_files[i]);
	for (int j=0; j<levels.numLevels(); j++) {
	  fprintf(stderr," %02d: %s\n",j,
		  levels.levelName(j).c_str());
	}
      }
    } else if ( op=="rtf" ) {
      RichText r("the quick brown fox, jumped over the lazy dog!");
      r.layout(100);
    } else {
      throw "bad test";
    }
  }

};


int npmain(int argc, char** argv)
{
  try {      
    App app(argc,argv);
    app.run();
  } catch ( const char* e ) {
    fprintf(stderr,"*** CAUGHT: %s",e);
  } 
  return 0;
}
