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
#if !defined(USE_HILDON) && !defined(WIN32)

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

/**
 * Include SDL, so that under Mac OS X it can rename my main()
 * function to SDL_main (else NP *will* crash on OS X).
 *
 * http://www.libsdl.org/faq.php?action=listentries&category=7#55
 **/
#include <SDL.h>

class OsFreeDesktop : public Os
{
 public:
  OsFreeDesktop()
    : m_fifo(NULL),
      m_cmdReady(false),
      m_cmdPos(0)
  {
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
};


Os* Os::get()
{
  static OsFreeDesktop os;
  return &os;
}

const char Os::pathSep = '/';

int main(int argc, char** argv)
{
  if ( ((OsFreeDesktop*)Os::get())->setupPipe(argc,argv) ) {
    npmain(argc,argv);
  }
}

#endif
