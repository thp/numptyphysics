/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2009, 2010 Tim Edmonds <numptyphysics@gmail.com>
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
#include "Script.h"
#include "Path.h"
#include "Scene.h"
#include <sstream>
#include <cstdio>

#include "thp_format.h"
#include "petals_log.h"

ScriptEntry::ScriptEntry( const std::string& str )
{
  char opc;
  if ( sscanf(str.c_str(), "%d,%c,%d,%d,%d,%d,%d",
	      &t, &opc, &stroke, &arg1, &arg2, &pt.x, &pt.y)==7 ) {
    switch (opc) {
    case 'n': op = OP_NEW; break;
    case 'd': op = OP_DELETE; break;
    case 'e': op = OP_EXTEND; break;
    case 'm': op = OP_MOVE; break;
    case 'a': op = OP_ACTIVATE; break;
    case 'p': op = OP_PAUSE; break;
    case 'g': op = OP_GOAL; break;
    default: LOG_WARNING("Bad script op");
    }
  } else {
    LOG_WARNING("Badly formed script entry");
  }
}


std::string ScriptEntry::asString()
{
  static const char opcodes[] = "ndemapg";
  std::stringstream s;
  s << t << "," << opcodes[op] << ","
    << stroke << "," << arg1 << "," << arg2 << ","
    << pt.x << "," << pt.y; 
  return thp::format("<np:event value=\"%s\" />", s.str().c_str());
}

std::string ScriptLog::asString( int i )
{
  if ( i < size() ) {
    return at(i).asString();
  }
  return std::string();
}

void ScriptLog::append( int tick, ScriptEntry::Op op, int stroke, 
		      int arg1, int arg2, const Vec2& pt )
{
  push_back( ScriptEntry( tick, op, stroke, arg1, arg2, pt ) );
}

void ScriptLog::append( const std::string& str ) 
{
  push_back( ScriptEntry(str) );
}



ScriptRecorder::ScriptRecorder()
  : m_running(false)
  , m_isPaused(false)
  , m_log(NULL)
{
}

void ScriptRecorder::start( ScriptLog* log ) 
{
  m_running = true;
  m_isPaused = false;
  m_log = log;
  m_log->clear();
  m_lastTick = 0;
}

void ScriptRecorder::stop()  
{ 
  if ( m_running ) {
    m_running = false; 
  }
}

void ScriptRecorder::tick(bool isPaused) 
{
  if ( m_running ) {
    m_lastTick++;
    if (isPaused != m_isPaused) {
      m_isPaused = isPaused;
      m_log->append( m_lastTick, ScriptEntry::OP_PAUSE, isPaused?1:0 );
    }
  }
}


void ScriptRecorder::newStroke( const Path& p, int colour, int attribs )
{
  if ( m_running )
    m_log->append( m_lastTick, ScriptEntry::OP_NEW, 0, colour, attribs, p[0] );
}


void ScriptRecorder::deleteStroke( int index )
{
  if ( m_running )
    m_log->append( m_lastTick, ScriptEntry::OP_DELETE, index );
}

void ScriptRecorder::extendStroke( int index, const Vec2& pt )
{
  if ( m_running )
    m_log->append( m_lastTick, ScriptEntry::OP_EXTEND, index, 0, 0, pt );
}

void ScriptRecorder::moveStroke( int index, const Vec2& pt )
{
  if ( m_running )
    m_log->append( m_lastTick, ScriptEntry::OP_MOVE, index, 0, 0, pt );
}

void ScriptRecorder::activateStroke( int index )
{
  if ( m_running )
    m_log->append( m_lastTick, ScriptEntry::OP_ACTIVATE, index );
}

void ScriptRecorder::goal( int goalNum )
{
  if ( m_running )
    m_log->append( m_lastTick, ScriptEntry::OP_GOAL, goalNum );
}



void ScriptPlayer::start( const ScriptLog* log, Scene* scene )
{
  m_playing = true;
  m_isPaused = false;
  m_log = log;
  m_index = 0;
  m_lastTick = 0;
  m_scene = scene;
  LOG_INFO("Start playback: %lu events", m_log->size());
}


void ScriptPlayer::stop()  
{ 
  m_playing = false; 
  m_log = NULL;
}

bool ScriptPlayer::isRunning() const
{
  return m_log && m_log->size() > 0 && m_playing; 
}

bool ScriptPlayer::tick() 
{
  if ( m_playing ) {
    m_lastTick++;

    while ( m_index < m_log->size()
	 && m_log->at(m_index).t <= m_lastTick ) {
      const ScriptEntry& e = m_log->at(m_index);
      switch (e.op) {
      case ScriptEntry::OP_NEW:
	m_scene->newStroke( Path(Path()&e.pt), e.arg1, e.arg2 );
	break;
      case ScriptEntry::OP_DELETE:
	m_scene->deleteStroke( m_scene->strokes()[e.stroke] );
	break;
      case ScriptEntry::OP_EXTEND:
	m_scene->extendStroke( m_scene->strokes()[e.stroke], e.pt );
	break;
      case ScriptEntry::OP_MOVE:
	m_scene->moveStroke( m_scene->strokes()[e.stroke], e.pt );
	break;
      case ScriptEntry::OP_ACTIVATE:
	m_scene->activateStroke( m_scene->strokes()[e.stroke] );
	break;
      case ScriptEntry::OP_PAUSE:
	m_isPaused = (e.stroke != 0);
	break;
      case ScriptEntry::OP_GOAL:
        // TODO
        break;
      }
      m_index++;
    }
    return m_isPaused;
  }
  return false;
}


