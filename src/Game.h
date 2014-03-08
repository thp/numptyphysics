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


class GameControl
{
public:
  GameControl() : m_quit(false),
		 m_edit( false ),
		 m_refresh( true ),
		 m_colour( 2 ),
                 m_clickMode(0),
		 m_strokeFixed( false ),
		 m_strokeSleep( false ),
		 m_strokeDecor( false ),
                 m_strokeRope( false ),
                 m_replaying( false ),
                 m_paused( false ),
                 m_levels(NULL),
                 m_level(0)
  {}
  virtual ~GameControl() {}
  virtual bool save( const char *file=NULL ) =0;
  virtual bool send() =0;
  virtual bool load( const char* file ) { return false; };
  virtual void gotoLevel( int l, bool replay=false ) =0;
  virtual void clickMode(int cm) =0;
  Levels& levels() { return *m_levels; }
  const GameStats& stats() { return m_stats; }
  bool  m_quit;
  bool  m_edit;
  bool  m_refresh;
  int   m_colour;
  int   m_clickMode;
  bool  m_strokeFixed;
  bool  m_strokeSleep;
  bool  m_strokeDecor;
  bool  m_strokeRope;
  bool  m_replaying;
  bool  m_paused;
  Levels*m_levels;
  int    m_level;
protected:
  GameStats         m_stats;
};


Widget * createGameLayer( Levels* levels, int width, int height );



#endif //GAME_H
