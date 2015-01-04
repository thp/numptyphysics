/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2008, 2009, 2010 Tim Edmonds <numptyphysics@gmail.com>
 * Coyright (c) 2014, 2015 Thomas Perl <m@thp.io>
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

#ifndef GAME_H
#define GAME_H

#include "Levels.h"

class Widget;
class Canvas;

struct GameStats
{
  GameStats() {reset(0);}
  int startTime;
  int endTime;
  int strokeCount;
  int pausedStrokes;
  int undoCount;
  int ropeCount;
  int pausedRopes;
  void reset(int t) {
    startTime = t;
    endTime = 0;
    strokeCount = 0;
    pausedStrokes = 0;
    undoCount = 0;
    ropeCount = 0;
    pausedRopes = 0;
  }
};

enum ClickMode {
    CLICK_MODE_DRAW_STROKE,
    CLICK_MODE_MOVE,
    CLICK_MODE_ERASE,
    CLICK_MODE_DRAW_JETSTREAM,
};


class GameControl
{
public:
  GameControl() : m_quit(false),
		 m_edit( false ),
		 m_colour( 2 ),
                 m_clickMode(CLICK_MODE_DRAW_STROKE),
		 m_strokeFixed( false ),
		 m_strokeSleep( false ),
		 m_strokeDecor( false ),
                 m_strokeRope( false ),
                 m_interactiveDraw( false ),
                 m_replaying( false ),
                 m_paused( false ),
                 m_levels(NULL),
                 m_level(0)
  {}
  virtual ~GameControl() {}
  virtual bool save( const char *file=NULL ) =0;
  virtual bool load( const char* file ) { return false; };
  virtual void gotoLevel(int l) = 0;
  virtual void clickMode(enum ClickMode cm) =0;
  virtual void toggleClickMode(enum ClickMode cm) = 0;
  Levels& levels() { return *m_levels; }
  const GameStats& stats() { return m_stats; }
  bool  m_quit;
  bool  m_edit;
  int   m_colour;
  enum ClickMode m_clickMode;
  bool  m_strokeFixed;
  bool  m_strokeSleep;
  bool  m_strokeDecor;
  bool  m_strokeRope;
  bool  m_interactiveDraw;
  bool  m_replaying;
  bool  m_paused;
  Levels*m_levels;
  int    m_level;
protected:
  GameStats         m_stats;
};


Widget * createGameLayer( Levels* levels, int width, int height );



#endif //GAME_H
