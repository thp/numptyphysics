/*
 * This file is part of NumptyPhysics
 * Copyright (C) 2009 Tim Edmonds
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
#ifndef SCRIPT_H
#define SCRIPT_H
#include "Array.h"
#include "Path.h"
#include <iostream>

class Scene;

struct ScriptEntry {
  enum Op {
    OP_NEW,
    OP_DELETE,
    OP_EXTEND,
    OP_MOVE,
    OP_ACTIVATE
  };

  Op   op;
  int  stroke;
  int  arg1;
  int  arg2;
  Vec2 pt;
  int  t;

  ScriptEntry( int _t, Op _op, int _stroke,
	     int _arg1, int _arg2, const Vec2& _pt ) 
  : t(_t), op(_op), stroke(_stroke),
    arg1(_arg1), arg2(arg2), pt(_pt)
  {}
  ScriptEntry() {};
  ScriptEntry( const std::string& str );
  std::string asString();
};


class ScriptLog : public Array<ScriptEntry>
{
public:
  std::string asString( int i );
  void append( int tick, ScriptEntry::Op op, int stroke=-1,
	       int arg1=-1, int arg2=-1, const Vec2& pt=Vec2(-1,-1) );
  void append( const std::string& str );
  using Array<ScriptEntry>::append;
};


class ScriptRecorder
{
public:
  ScriptRecorder();
  void start(ScriptLog* log);
  void stop();
  void tick();
  void newStroke( const Path& p, int colour, int attribs );
  void deleteStroke( int index );
  void extendStroke( int index, const Vec2& pt );
  void moveStroke( int index, const Vec2& pt );
  void activateStroke( int index );

  ScriptLog* getLog() { return m_log; }

private:
  bool          m_running;
  ScriptLog      *m_log;
  int 		m_lastTick;
};


class ScriptPlayer
{
public:

  void start( const ScriptLog* log, Scene* scene );
  bool isRunning();
  void stop();
  void tick();

private:
  bool           m_playing;
  const ScriptLog* m_log;
  Scene         *m_scene;
  int            m_index;
  int  		 m_lastTick;
};



#endif //SCRIPT_H
