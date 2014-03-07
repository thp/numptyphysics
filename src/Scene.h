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
#include "Path.h"
#include "Canvas.h"
#include "Script.h"

#include <string>
#include <fstream>
#include <vector>


class Stroke;
class b2World;
class Accelerometer;

typedef enum {
  ATTRIB_DUMMY = 0,
  ATTRIB_GROUND = 1,
  ATTRIB_TOKEN = 2,
  ATTRIB_GOAL = 4,
  ATTRIB_DECOR = 8,
  ATTRIB_SLEEPING = 16,
  ATTRIB_HIDDEN = 32,
  ATTRIB_DELETED = 64,
  ATTRIB_CLASSBITS = ATTRIB_TOKEN | ATTRIB_GOAL,
  ATTRIB_UNJOINABLE = ATTRIB_DECOR | ATTRIB_HIDDEN | ATTRIB_DELETED
} Attribute;


class Scene : private b2ContactListener
{
public:

  Scene( bool noWorld=false );
  ~Scene();

  Stroke* newStroke( const Path& p, int colour, int attributes );
  bool deleteStroke( Stroke *s );
  void extendStroke( Stroke* s, const Vec2& pt );
  void moveStroke( Stroke* s, const Vec2& origin );
  bool activateStroke( Stroke *s );
  void getJointCandidates( Stroke* s, Path& pts );

  int numStrokes() {
    return m_strokes.size();
  }

  std::vector<Stroke*>& strokes() {
    return m_strokes;
  }

  void step( bool isPaused=false );
  bool isCompleted();
  void draw( Canvas& canvas, const Rect& area );
  void reset( Stroke* s=NULL,  bool purgeUnprotected=false );
  Stroke* strokeAtPoint( const Vec2 pt, float32 max );
  void clear();

  void setGravity( const b2Vec2& g );
  void setGravity( const std::string& s );

  bool load( unsigned char *buf, int bufsize );
  bool load( std::istream& in );
  void start( bool replay=false );
  void protect( int n=-1 );
  bool save( const std::string& file, bool saveLog=false );

  ScriptLog* getLog() { return &m_log; }
  const ScriptPlayer* replay() { return &m_player; }
private:
  void resetWorld();
  bool activate( Stroke *s );
  void activateAll();
  void createJoints( Stroke *s );

  // b2ContactListener callback when a new contact is detected
  virtual void Add(const b2ContactPoint* point) ;


  b2World        *m_world;
  std::vector<Stroke*>  m_strokes;
  std::vector<Stroke*>  m_deletedStrokes;
  std::string     m_title, m_author, m_bg;
  ScriptLog       m_log;
  ScriptRecorder  m_recorder;
  ScriptPlayer    m_player;
  Image          *m_bgImage;
  static Image   *g_bgImage;
  int             m_protect;
  b2Vec2          m_gravity;
  b2Vec2          m_currentGravity;
  bool            m_dynamicGravity;
  Accelerometer  *m_accelerometer;
};



class Transform
{
public:
  Transform( float32 scale, float32 rotation, const Vec2& translation );
  void set( float32 scale, float32 rotation, const Vec2& translation );

  inline void transform( const Path& pin, Path& pout ) {
    pout = pin;
    if ( !m_bypass ) {
      pout.rotate( m_rot );
      pout.translate( m_pos );
    }
  }
  inline void transform( Vec2& vec ) {
    if ( !m_bypass ) {
      vec = Vec2( b2Mul( m_rot, vec ) ) + m_pos;
    }
  }
  inline void inverseTransform( Vec2& vec ) {
    if ( !m_bypass ) {
      vec = Vec2( b2Mul( m_invrot, vec-m_pos ) );
    }
  }
private:
  Transform() {}
  bool m_bypass;
  b2Mat22 m_rot;
  b2Mat22 m_invrot;
  Vec2 m_pos;
};

extern Transform worldToScreen;

extern void configureScreenTransform( int w, int h );
