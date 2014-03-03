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
#include "Scene.h"
#include "Accelerometer.h"

#include <sstream>
#include <fstream>


Transform::Transform( float32 scale, float32 rotation, const Vec2& translation )
{
  set( scale, rotation, translation );
}

void Transform::set( float32 scale, float32 rotation, const Vec2& translation )
{
  if ( scale==0.0f && rotation==0.0f && translation==Vec2(0,0) ) {
    m_bypass = true;
  } else {
    m_rot.Set( rotation );
    m_pos = translation;
    m_rot.col1.x *= scale;
    m_rot.col1.y *= scale;
    m_rot.col2.x *= scale;
    m_rot.col2.y *= scale;
    m_invrot = m_rot.Invert();
    m_bypass = false;
  }
}

Transform worldToScreen( 0.5f, M_PI/2, Vec2(240,0) );

void configureScreenTransform( int w, int h )
{
  SCREEN_WIDTH = w;
  SCREEN_HEIGHT = h;
  FULLSCREEN_RECT = Rect(0,0,w-1,h-1);
  if ( w==WORLD_WIDTH && h==WORLD_HEIGHT ) { //unity
    worldToScreen.set( 0.0f, 0.0f, Vec2(0,0) );
  } else {
    float rot = 0.0f;
    Vec2 tr(0,0);
    if ( h > w ) { //portrait
      rot = M_PI/2;
      tr = Vec2( w, 0 );
      b2Swap( h, w );
    }
    float scalew = (float)w/(float)WORLD_WIDTH;
    float scaleh = (float)h/(float)WORLD_HEIGHT;
    if ( scalew < scaleh ) {
      worldToScreen.set( scalew, rot, tr );
    } else {
      worldToScreen.set( scaleh, rot, tr );
    }
  }
}


struct Joint
{
  Joint( Stroke *j1, Stroke* j2, unsigned char e )
    : joiner(j1), joinee(j2), end(e) {}
  Stroke *joiner;
  Stroke *joinee;
  unsigned char end; //of joiner
};

class Stroke
{
public:

private:
  struct JointDef : public b2RevoluteJointDef
  {
    JointDef( b2Body* b1, b2Body* b2, const b2Vec2& pt )
    {
      Initialize( b1, b2, pt );
      maxMotorTorque = 10.0f;
      motorSpeed = 0.0f;
      enableMotor = true;
    }
  };

  struct BoxDef : public b2PolygonDef
  {
    void init( const Vec2& p1, const Vec2& p2, int attr )
    {
      b2Vec2 barOrigin = p1;
      b2Vec2 bar = p2 - p1;
      bar *= 1.0f/PIXELS_PER_METREf;
      barOrigin *= 1.0f/PIXELS_PER_METREf;;
      SetAsBox( bar.Length()/2.0f, 0.1f,
		0.5f*bar + barOrigin, vec2Angle( bar ));
      //      SetAsBox( bar.Length()/2.0f+b2_toiSlop, b2_toiSlop*2.0f,
      //	0.5f*bar + barOrigin, vec2Angle( bar ));
      friction = 0.3f;
      if ( attr & ATTRIB_GROUND ) {
	density = 0.0f;
      } else if ( attr & ATTRIB_GOAL ) {
	density = 100.0f;
      } else if ( attr & ATTRIB_TOKEN ) {
	density = 3.0f;
	friction = 0.1f;
      } else {
	density = 5.0f;
      }
      restitution = 0.2f;
    }
  };

public:
  Stroke( const Path& path )
    : m_rawPath(path)
  {
    m_colour = brushColours[DEFAULT_BRUSH];
    m_attributes = 0;
    m_origin = m_rawPath.point(0);
    m_rawPath.translate( -m_origin );
    reset();
  }  

  Stroke( const std::string& str ) 
  {
    int col = 0;
    m_colour = brushColours[DEFAULT_BRUSH];
    m_attributes = 0;
    m_origin = Vec2(400,240);
    reset();
    const char *s = str.c_str();
    while ( *s && *s!=':' && *s!='\n' ) {
      switch ( *s ) {
      case 't': setAttribute( ATTRIB_TOKEN ); break;	
      case 'g': setAttribute( ATTRIB_GOAL ); break;	
      case 'f': setAttribute( ATTRIB_GROUND ); break;
      case 's': setAttribute( ATTRIB_SLEEPING ); break;
      case 'd': setAttribute( ATTRIB_DECOR ); break;
      default:
	if ( *s >= '0' && *s <= '9' ) {
	  col = col*10 + *s -'0';
	}
	break;
      }
      s++;
    }
    if ( col >= 0 && col < NUM_BRUSHES ) {
      m_colour = brushColours[col];
    }
    if ( *s++ == ':' ) {
      m_rawPath = Path(s);
    }
    if ( m_rawPath.size() < 2 ) {
      throw "invalid stroke def";
    }
    //fprintf(stderr,"created stroke with %d points\n",m_rawPath.size());
    m_origin = m_rawPath.point(0);
    m_rawPath.translate( -m_origin );
    setAttribute( ATTRIB_DUMMY );
  }

  void reset( b2World* world=NULL )
  {
    if (m_body && world) {
      world->DestroyBody( m_body );
    }
    m_body = NULL;
    m_xformAngle = 7.0f;
    m_drawnBbox.tl = m_origin;
    m_drawnBbox.br = m_origin;
    m_jointed[0] = m_jointed[1] = false;
    m_shapePath = m_rawPath;
    m_hide = 0;
    m_drawn = false;
  }

  std::string asString()
  {
    std::stringstream s;
    s << 'S';
    if ( hasAttribute(ATTRIB_TOKEN) )    s<<'t';
    if ( hasAttribute(ATTRIB_GOAL) )     s<<'g';
    if ( hasAttribute(ATTRIB_GROUND) )   s<<'f';
    if ( hasAttribute(ATTRIB_SLEEPING) ) s<<'s';
    if ( hasAttribute(ATTRIB_DECOR) )    s<<'d';
    for ( int i=0; i<NUM_BRUSHES; i++ ) {
      if ( m_colour==brushColours[i] )  s<<i;
    }
    s << ":";
    Path opath = m_rawPath;
    opath.translate(m_origin);
    for ( int i=0; i<opath.size(); i++ ) {
      const Vec2& p = opath.point(i);
      s <<' '<< p.x << ',' << p.y; 
    }
    s << std::endl;
    return s.str();
  }

  void setAttribute( Attribute a )
  {
    m_attributes |= a;
    if ( m_attributes & ATTRIB_TOKEN )     m_colour = brushColours[RED_BRUSH];
    else if ( m_attributes & ATTRIB_GOAL ) m_colour = brushColours[YELLOW_BRUSH];
  }

  void clearAttribute( Attribute a )
  {
    m_attributes &= ~a;
  }

  bool hasAttribute( Attribute a )
  {
    return (m_attributes&a) != 0;
  }
  void setColour( int c ) 
  {
    m_colour = c;
  }

  void createBodies( b2World& world )
  {
    process();
    if ( hasAttribute( ATTRIB_DECOR ) ){
      return; //decorators have no physical embodiment
    }
    int n = m_shapePath.numPoints();
    if ( n > 1 ) {
      b2BodyDef bodyDef;
      bodyDef.position = m_origin;
      bodyDef.position *= 1.0f/PIXELS_PER_METREf;
      bodyDef.userData = this;
      if ( m_attributes & ATTRIB_SLEEPING ) {
	bodyDef.isSleeping = true;
      }
      m_body = world.CreateBody( &bodyDef );
      for ( int i=1; i<n; i++ ) {
	BoxDef boxDef;
	boxDef.init( m_shapePath.point(i-1),
		     m_shapePath.point(i),
		     m_attributes );
	m_body->CreateShape( &boxDef );
      }
      m_body->SetMassFromShapes();

    }
    transform();
  }

  void determineJoints( Stroke* other, Array<Joint>& joints )
  {
    if ( (m_attributes&ATTRIB_CLASSBITS)
	 != (other->m_attributes&ATTRIB_CLASSBITS)
	 || hasAttribute(ATTRIB_GROUND)
	 || hasAttribute(ATTRIB_UNJOINABLE)
	 || other->hasAttribute(ATTRIB_UNJOINABLE)) {
      // cannot joint goals or tokens to other things
      // and no point jointing ground endpts
      return;
    } 

    transform();
    for ( unsigned char end=0; end<2; end++ ) {
      if ( !m_jointed[end] ) {
	const Vec2& p = m_xformedPath.endpt(end);
	if ( other->distanceTo( p ) <= JOINT_TOLERANCE ) {
	  joints.append( Joint(this,other,end) );
	}
      }
    }
  }

  void join( b2World* world, Stroke* other, unsigned char end )
  {
    if ( !m_jointed[end] ) {
      b2Vec2 p = m_xformedPath.endpt( end );
      p *= 1.0f/PIXELS_PER_METREf;
      JointDef j( m_body, other->m_body, p );
      world->CreateJoint( &j );
      m_jointed[end] = true;
    }
  }

  bool maybeCreateJoint( b2World& world, Stroke* other )
  {
    if ( (m_attributes&ATTRIB_CLASSBITS)
	 != (other->m_attributes&ATTRIB_CLASSBITS) ) {
      return false; // can only joint matching classes
    } else if ( hasAttribute(ATTRIB_GROUND) ) {
      return true; // no point jointing grounds
    } else if ( m_body && other->body() ) {
      transform();
      int n = m_xformedPath.numPoints();
      for ( int end=0; end<2; end++ ) {
	if ( !m_jointed[end] ) {
	  const Vec2& p = m_xformedPath.point( end ? n-1 : 0 );
	  if ( other->distanceTo( p ) <= JOINT_TOLERANCE ) {
	    //printf("jointed end %d d=%f\n",end,other->distanceTo( p ));
	    b2Vec2 pw = p;
	    pw *= 1.0f/PIXELS_PER_METREf;
	    JointDef j( m_body, other->m_body, pw );
	    world.CreateJoint( &j );
	    m_jointed[end] = true;
	  }
	}
      }
    }
    if ( m_body ) {
      return m_jointed[0] && m_jointed[1];
    }
    return true; ///nothing to do
  }

  void draw( Canvas& canvas, bool drawJoints=false )
  {
    if ( m_hide < HIDE_STEPS ) {
      int colour = canvas.makeColour(m_colour);
      bool thick = (canvas.width() > 400);
      transform();
      canvas.drawPath( m_screenPath, colour, thick );
      m_drawn = true;
      
      if ( drawJoints ) {
	int jointcolour = canvas.makeColour(0xff0000);
	for ( int e=0; e<2; e++ ) {
	  if (m_jointed[e]) {
	    const Vec2& pt = m_screenPath.endpt(e);
	    //canvas.drawPixel( pt.x, pt.y, jointcolour );
	    //canvas.drawRect( pt.x-1, pt.y-1, 3, 3, jointcolour );
	    canvas.drawRect( pt.x-1, pt.y, 3, 1, jointcolour );
	    canvas.drawRect( pt.x, pt.y-1, 1, 3, jointcolour );
	  }
	}
      }
    }
    m_drawnBbox = m_screenBbox;
  }

  void addPoint( const Vec2& pp ) 
  {
    Vec2 p = pp; p -= m_origin;
    if ( p == m_rawPath.point( m_rawPath.numPoints()-1 ) ) {
    } else {
      m_rawPath.append( p );
      m_drawn = false;
    }
  }

  void origin( const Vec2& p ) 
  {
    // todo 
    if ( m_body ) {
      b2Vec2 pw = p;
      pw *= 1.0f/PIXELS_PER_METREf;
      m_body->SetXForm( pw, m_body->GetAngle() );
    }
    m_origin = p;
    m_drawn = false;
  }

  b2Body* body() { return m_body; }

  float32 distanceTo( const Vec2& pt )
  {
    float32 best = 100000.0;
    transform();
    for ( int i=1; i<m_xformedPath.numPoints(); i++ ) {    
      Segment s( m_xformedPath.point(i-1), m_xformedPath.point(i) );
      float32 d = s.distanceTo( pt );
      //printf("  d[%d]=%f %d,%d\n",i,d,m_rawPath.point(i-1).x,m_rawPath.point(i-1).y);
      if ( d < best ) {
        best = d;
      }
    }
    return best;
  }

  Rect screenBbox() 
  {
    transform();
    return m_screenBbox;
  }

  Rect lastDrawnBbox() 
  {
    return m_drawnBbox;
  }

  Rect worldBbox() 
  {
    return m_xformedPath.bbox();
  }

  void hide()
  {
    if ( m_hide==0 ) {
      m_hide = 1;
      
      if (m_body) {
	// stash the body where no-one will find it
	m_body->SetXForm( b2Vec2(0.0f,SCREEN_HEIGHT*2.0f), 0.0f );
	m_body->SetLinearVelocity( b2Vec2(0.0f,0.0f) );
	m_body->SetAngularVelocity( 0.0f );
      }
    }
  }

  bool hidden()
  {
    return m_hide >= HIDE_STEPS;
  }

  int numPoints()
  {
    return m_rawPath.numPoints();
  }

  const Vec2& endpt( unsigned char end ) 
  {
    return m_xformedPath.endpt(end);
  }

private:
  static float32 vec2Angle( b2Vec2 v ) 
  {
    return b2Atan2(v.y, v.x);
  } 

  void process()
  {
    float32 thresh = SIMPLIFY_THRESHOLDf;
    m_rawPath.simplify( thresh );
    m_shapePath = m_rawPath;
    //fprintf(stderr,"simplified stroke to %d points\n",m_rawPath.size());

    while ( m_shapePath.numPoints() > MULTI_VERTEX_LIMIT ) {
      thresh += SIMPLIFY_THRESHOLDf;
      m_shapePath.simplify( thresh );
    }
  }

  bool transform()
  {
    // distinguish between xformed raw and shape path as needed
    if ( m_hide ) {
      if ( m_hide < HIDE_STEPS ) {
	//printf("hide %d\n",m_hide);
	Vec2 o = m_screenBbox.centroid();
	m_screenPath -= o;
	m_screenPath.scale( 0.99 );
	m_screenPath += o;
	m_screenBbox = m_screenPath.bbox();      
	m_hide++;
	return true;
      }
    } else if ( m_body ) {
      if ( hasAttribute( ATTRIB_DECOR ) ) {
	return false; // decor never moves
      } else if ( hasAttribute( ATTRIB_GROUND )	   
		  && m_xformAngle == m_body->GetAngle() ) {
	return false; // ground strokes never move.
      } else if ( m_xformAngle != m_body->GetAngle() 
	   ||  ! (m_xformPos == m_body->GetPosition()) ) {
	//printf("transform stroke - rot or pos\n");
	b2Mat22 rot( m_body->GetAngle() );
	b2Vec2 orig = PIXELS_PER_METREf * m_body->GetPosition();
	m_xformedPath = m_rawPath;
	m_xformedPath.rotate( rot );
	m_xformedPath.translate( Vec2(orig) );
	m_xformAngle = m_body->GetAngle();
	m_xformPos = m_body->GetPosition();
	worldToScreen.transform( m_xformedPath, m_screenPath );
	m_screenBbox = m_screenPath.bbox();      
      } else {
	//printf("transform none\n");
	return false;
      }
    } else {
      //printf("transform no body\n");
      m_xformedPath = m_rawPath;
      m_xformedPath.translate( m_origin );
      worldToScreen.transform( m_xformedPath, m_screenPath );
      m_screenBbox = m_screenPath.bbox();      
      return !hasAttribute(ATTRIB_DECOR);
    }
    return true;
  }

  Path      m_rawPath;
  int       m_colour;
  int       m_attributes;
  Vec2      m_origin;
  Path      m_shapePath;
  Path      m_xformedPath;
  Path      m_screenPath;
  float32   m_xformAngle;
  b2Vec2    m_xformPos;
  Rect      m_screenBbox;
  Rect      m_drawnBbox;
  bool      m_drawn;
  b2Body*   m_body;
  bool      m_jointed[2];
  int       m_hide;
};


Scene::Scene( bool noWorld )
  : m_world( NULL ),
    m_bgImage( NULL ),
    m_protect( 0 ),
    m_gravity(0.0f, 0.0f),
    m_dynamicGravity(false),
    m_accelerometer(Os::get()->getAccelerometer())
{
  if ( !noWorld ) {
    resetWorld();
  }
}

Scene::~Scene()
{
  clear();
  if ( m_world ) {
    delete m_world;
  }
}

void Scene::resetWorld()
{
  const b2Vec2 gravity(0.0f, GRAVITY_ACCELf*PIXELS_PER_METREf/GRAVITY_FUDGEf);
  delete m_world;

  b2AABB worldAABB;
  worldAABB.lowerBound.Set(-100.0f, -100.0f);
  worldAABB.upperBound.Set(100.0f, 100.0f);
    
  bool doSleep = true;
  m_world = new b2World(worldAABB, gravity, doSleep);
  m_world->SetContactListener( this );
}

Stroke* Scene::newStroke( const Path& p, int colour, int attribs ) {
  Stroke *s = new Stroke(p);
  s->setAttribute( (Attribute)attribs );

  switch ( colour ) {
  case 0: s->setAttribute( ATTRIB_TOKEN ); break;
  case 1: s->setAttribute( ATTRIB_GOAL ); break;
  default: s->setColour( brushColours[colour] ); break;
  }
  m_strokes.append( s );
  m_recorder.newStroke( p, colour, attribs );
  return s;
}

bool Scene::deleteStroke( Stroke *s ) {
  if ( s ) {
    int i = m_strokes.indexOf(s);
    if ( i >= m_protect ) {
	reset(s);
	m_strokes.erase( i );
	m_deletedStrokes.append( s );
	m_recorder.deleteStroke( i );
	return true;
    }
  }
  return false;
}


void Scene::extendStroke( Stroke* s, const Vec2& pt )
{
  if ( s ) {
    int i = m_strokes.indexOf(s);
    if ( i >= m_protect ) {
      s->addPoint( pt );
      m_recorder.extendStroke( i, pt );
    }
  }
}

void Scene::moveStroke( Stroke* s, const Vec2& origin )
{
  if ( s ) {
    int i = m_strokes.indexOf(s);
    if ( i >= m_protect ) {
      s->origin( origin );
      m_recorder.moveStroke( i, origin );
    }
  }
}
	

bool Scene::activateStroke( Stroke *s )
{
  bool result = activate(s);
  m_recorder.activateStroke( m_strokes.indexOf(s) );
  return result;
}

void Scene::getJointCandidates( Stroke* s, Path& pts )
{
  Array<Joint> joints;
  for ( int j=m_strokes.size()-1; j>=0; j-- ) {      
    if ( s != m_strokes[j] ) {
      s->determineJoints( m_strokes[j], joints );
      m_strokes[j]->determineJoints( s, joints );
    }
  }
  for ( int j=joints.size()-1; j>=0; j-- ) {
    pts.append( joints[j].joiner->endpt(joints[j].end) );
  }
}

bool Scene::activate( Stroke *s )
{
  if ( s->numPoints() > 1 ) {
    s->createBodies( *m_world );
    createJoints( s );
    return true;
  }
  return false;
}

void Scene::activateAll()
{
  for ( int i=0; i < m_strokes.size(); i++ ) {
    m_strokes[i]->createBodies( *m_world );
  }
  for ( int i=0; i < m_strokes.size(); i++ ) {
    createJoints( m_strokes[i] );
  }
}

void Scene::createJoints( Stroke *s )
{
  if ( s->body()==NULL ) {
    return;
  }
  Array<Joint> joints;
  for ( int j=m_strokes.size()-1; j>=0; j-- ) {      
    if ( s != m_strokes[j] && m_strokes[j]->body() ) {
	//printf("try join to %d\n",j);
      s->determineJoints( m_strokes[j], joints );
      m_strokes[j]->determineJoints( s, joints );
      for ( int i=0; i<joints.size(); i++ ) {
	joints[i].joiner->join( m_world, joints[i].joinee, joints[i].end );
      }
      joints.empty();
    }
  }    
}

void Scene::step( bool isPaused )
{
  m_recorder.tick(isPaused);
  isPaused |= m_player.tick();

  if ( !isPaused ) {
    if (m_accelerometer && m_dynamicGravity) {
      float32 gx, gy, gz;
      if ( m_accelerometer->poll( gx, gy, gz ) ) {
	
	if (m_dynamicGravity || gx*gx+gy*gy > 1.2*1.2)  {
	  //fprintf(stderr,"dynamic grav = %f,%f\n", gx, gy );
	  const float32 factor = GRAVITY_ACCELf*PIXELS_PER_METREf/GRAVITY_FUDGEf;
	  m_currentGravity = b2Vec2( m_gravity.x + gx*factor, 
				     m_gravity.y + gy*factor );
	  m_world->SetGravity( m_currentGravity );
	} else if (!(m_currentGravity == m_gravity)) {
	  m_currentGravity += m_gravity;
	  m_currentGravity *= 0.5;
	  m_world->SetGravity( m_currentGravity );
	}
	//TODO record gravity
      }
    }

    m_world->Step( ITERATION_TIMESTEPf, SOLVER_ITERATIONS );
    // clean up delete strokes
    for ( int i=0; i< m_strokes.size(); i++ ) {
      if ( m_strokes[i]->hasAttribute(ATTRIB_DELETED) ) {
	m_strokes[i]->clearAttribute(ATTRIB_DELETED);
	m_strokes[i]->hide();
      }	   
    }
    // check for token respawn
    for ( int i=0; i < m_strokes.size(); i++ ) {
      if ( m_strokes[i]->hasAttribute( ATTRIB_TOKEN )
	   && !BOUNDS_RECT.intersects( m_strokes[i]->worldBbox() ) ) {
	reset( m_strokes[i] );
	activate( m_strokes[i] );	  
      }
    }
  }
}

// b2ContactListener callback when a new contact is detected
void Scene::Add(const b2ContactPoint* point) 
{     
  // check for completion
  //if (c->GetManifoldCount() > 0) {
  Stroke* s1 = (Stroke*)point->shape1->GetBody()->GetUserData();
  Stroke* s2 = (Stroke*)point->shape2->GetBody()->GetUserData();
  if ( s1 && s2 ) {
    if ( s2->hasAttribute(ATTRIB_TOKEN) ) {
	b2Swap( s1, s2 );
    }
    if ( s1->hasAttribute(ATTRIB_TOKEN) 
	   && s2->hasAttribute(ATTRIB_GOAL) ) {
	s2->setAttribute(ATTRIB_DELETED);
	m_recorder.goal(1);
    }
  }
}

bool Scene::isCompleted()
{
  for ( int i=0; i < m_strokes.size(); i++ ) {
    if ( m_strokes[i]->hasAttribute( ATTRIB_GOAL )
	   && !m_strokes[i]->hidden() ) {
	return false;
    }
  }
  //printf("completed!\n");
  return true;
}

void Scene::draw( Canvas& canvas, const Rect& area )
{
  if ( m_bgImage ) {
    canvas.setBackground( m_bgImage );
  } else {
    canvas.setBackground( 0 );
  }
  for ( int i=0; i<m_strokes.size(); i++ ) {
	m_strokes[i]->draw( canvas );
  }
  while ( m_deletedStrokes.size() ) {
    delete m_deletedStrokes[0];
    m_deletedStrokes.erase(0);
  }
}

void Scene::reset( Stroke* s, bool purgeUnprotected )
{
  while ( purgeUnprotected && m_strokes.size() > m_protect ) {
    m_strokes[m_strokes.size()-1]->reset(m_world);
    m_strokes.erase( m_strokes.size()-1 );
  }
  for ( int i=0; i<m_strokes.size(); i++ ) {
    if (s==NULL || s==m_strokes[i]) {
	m_strokes[i]->reset(m_world);
    }
  }
}

Stroke* Scene::strokeAtPoint( const Vec2 pt, float32 max )
{
  Stroke* best = NULL;
  for ( int i=0; i<m_strokes.size(); i++ ) {
    float32 d = m_strokes[i]->distanceTo( pt );
    //printf("stroke %d dist %f\n",i,d);
    if ( d < max ) {
	max = d;
	best = m_strokes[i];
    }
  }
  return best;
}

void Scene::clear()
{
  reset();
  while ( m_strokes.size() ) {
    delete m_strokes[0];
    m_strokes.erase(0);
  }
  while ( m_deletedStrokes.size() ) {
    delete m_deletedStrokes[0];
    m_deletedStrokes.erase(0);
  }
  if ( m_world ) {
    //step is required to actually destroy bodies and joints
    m_world->Step( ITERATION_TIMESTEPf, SOLVER_ITERATIONS );
  }
  m_log.empty();
}

void Scene::setGravity( const b2Vec2& g )
{
  m_gravity = m_currentGravity = g;
  if (m_world) {
    m_world->SetGravity( m_gravity );
  }
}

void Scene::setGravity( const std::string& s )
{
  for (int i=0; i<s.find(':'); i++) {
    switch (s[i]) {
    case 'd': m_dynamicGravity = true; break;
    }
  }

  std::string vector = s.substr(s.find(':')+1);
  float32 x,y;      
  if ( sscanf( vector.c_str(), "%f,%f", &x, &y )==2) {
    if ( m_world ) {
	b2Vec2 g(x,y);
	g *= PIXELS_PER_METREf/GRAVITY_FUDGEf;
	setGravity( g );
    }
  } else {
    fprintf(stderr,"invalid gravity vector [%s]\n",vector.c_str());
  }
}

bool Scene::load( unsigned char *buf, int bufsize )
{
  std::string s( (const char*)buf, bufsize );
  std::stringstream in( s, std::ios::in );
  return load( in );
}

bool Scene::load( const std::string& file )
{
  std::ifstream in( file.c_str(), std::ios::in );
  return load( in ); 
}

bool Scene::load( std::istream& in )
{
  clear();
  resetWorld();
  m_dynamicGravity = false;
  if ( g_bgImage==NULL ) {
    g_bgImage = Image::fromFile("paper.png");
  }
  m_bgImage = g_bgImage;
  std::string line;
  while ( !in.eof() ) {
    getline( in, line );
    parseLine( line );
  }
  protect();
  printf("loaded log=%d\n",m_log.size());
  return true;
}


void Scene::start( bool replay )
{
  activateAll();
  if ( replay ) {
    m_recorder.stop();
    m_player.start( &m_log, this );
  } else {
    m_player.stop();
    m_recorder.start( &m_log );
  }
}


bool Scene::parseLine( const std::string& line )
{
  try {
    switch( line[0] ) {
    case 'T': m_title = line.substr(line.find(':')+1);  return true;
    case 'B': m_bg = line.substr(line.find(':')+1);     return true;
    case 'A': m_author = line.substr(line.find(':')+1); return true;
    case 'S': m_strokes.append( new Stroke(line) );     return true;
    case 'G': setGravity(line);                         return true;
    case 'E': m_log.append(line.substr(line.find(':')+1));return true;
    }
  } catch ( const char* e ) {
    printf("Stroke error: %s\n",e);
  }
  return false;
}

void Scene::protect( int n )
{
  m_protect = (n==-1 ? m_strokes.size() : n );
}

bool Scene::save( const std::string& file, bool saveLog )
{
  printf("saving to %s\n",file.c_str());
  std::ofstream o( file.c_str(), std::ios::out );
  if ( o.is_open() ) {
    o << "Title: "<<m_title<<std::endl;
    o << "Author: "<<m_author<<std::endl;
    o << "Background: "<<m_bg<<std::endl;
    for ( int i=0; i<m_strokes.size() && (!saveLog || i<m_protect); i++ ) {
	o << m_strokes[i]->asString();
    }

    if ( saveLog ) {      
      for ( int i=0; i<m_log.size(); i++ ) {
	o << "E: " << m_log.asString( i ) <<std::endl;
      }
    }

    o.close();
    return true;
  } else {
    return false;
  }
}


Image *Scene::g_bgImage = NULL;

