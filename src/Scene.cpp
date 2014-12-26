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
#include "Config.h"
#include "Scene.h"
#include "Accelerometer.h"
#include "Colour.h"
#include "Stroke.h"

#include "tinyxml2.h"

#include <vector>
#include <list>
#include <algorithm>


Scene::Scene( bool noWorld )
  : m_world( NULL ),
    m_bgImage( NULL ),
    m_protect( 0 ),
    m_gravity(0.0f, 0.0f),
    m_dynamicGravity(false),
    m_accelerometer(Os::get()->getAccelerometer()),
    m_step(0)
  , m_color_rects()
  , m_interactions()
{
    if ( g_bgImage==NULL ) {
        g_bgImage = new Image("paper.png");
    }
    m_bgImage = g_bgImage;

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
  default: s->setColour( NP::Colour::values[colour] ); break;
  }
  m_strokes.push_back( s );
  m_recorder.newStroke( p, colour, attribs );
  return s;
}

bool Scene::deleteStroke( Stroke *s ) {
  if ( s ) {
    int i = indexOf(m_strokes, s);
    if ( i >= m_protect ) {
	reset(s);
	m_strokes.erase(std::find(m_strokes.begin(), m_strokes.end(), s));
	m_deletedStrokes.push_back(s);
	m_recorder.deleteStroke(i);
	return true;
    }
  }
  return false;
}


void Scene::extendStroke( Stroke* s, const Vec2& pt )
{
  if ( s ) {
    int i = indexOf(m_strokes, s);
    if ( i >= m_protect ) {
      s->addPoint( pt );
      m_recorder.extendStroke( i, pt );
    }
  }
}

void Scene::moveStroke( Stroke* s, const Vec2& origin )
{
  if ( s ) {
    int i = indexOf(m_strokes, s);
    if ( i >= m_protect ) {
      s->origin( origin );
      m_recorder.moveStroke( i, origin );
    }
  }
}
	

bool Scene::activateStroke( Stroke *s )
{
  bool result = activate(s);
  m_recorder.activateStroke( indexOf(m_strokes, s) );
  return result;
}

std::list<Vec2> Scene::getJointCandidates(Stroke *s)
{
    std::vector<Joint> joints;
    for (auto &stroke: m_strokes) {
        if (s == stroke) {
            continue;
        }
        s->determineJoints(stroke, joints);
        stroke->determineJoints(s, joints);
    }

    std::list<Vec2> result;
    for (auto &joint: joints) {
        result.push_front(joint.joiner->endpt(joint.end));
    }
    return result;
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
  std::vector<Joint> joints;
  for ( int j=m_strokes.size()-1; j>=0; j-- ) {      
    if ( s != m_strokes[j] && m_strokes[j]->body() ) {
	//printf("try join to %d\n",j);
      s->determineJoints( m_strokes[j], joints );
      m_strokes[j]->determineJoints( s, joints );
      for ( int i=0; i<joints.size(); i++ ) {
	joints[i].joiner->join( m_world, joints[i].joinee, joints[i].end );
      }
      joints.clear();
    }
  }    
}

void Scene::step( bool isPaused )
{
  m_recorder.tick(isPaused);
  isPaused |= m_player.tick();

  if ( (m_step++ >= m_strokes.size()) && !isPaused ) {
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

    // Update bounding boxes for interactive elements
    m_color_rects = calcColorRects();
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

void Scene::draw(Canvas &canvas, bool everything)
{
    if (m_bgImage) {
        canvas.drawImage(*m_bgImage);
    }

    int i = 0;
    const int fade_duration = 50;
    for (auto &stroke: m_strokes) {
        int a = 0;
        if (everything || m_step > i + fade_duration) {
            a = 255;
        } else if (m_step > i) {
            a = 255 * float(m_step - i) / fade_duration;
        }
        stroke->draw(canvas, a);
        i++;
    }

    clearWithDelete(m_deletedStrokes);

    for (auto &kv: m_color_rects) {
        canvas.drawRect(kv.second, kv.first, true, 128);
    }
}

bool Scene::interact(const Vec2 &pos)
{
    for (auto &kv: m_color_rects) {
        if (kv.second.contains(pos)) {
            if (m_interactions.handle(kv.first)) {
                return true;
            }
        }
    }

    return false;
}

std::map<int,Rect> Scene::calcColorRects()
{
    std::map<int,Rect> result;

    std::map<int,std::list<Stroke *>> strokesMap;
    for (auto &stroke: m_strokes) {
        if (!stroke->hasAttribute(ATTRIB_INTERACTIVE)) {
            continue;
        }

        strokesMap[NP::Colour::toIndex(stroke->colour())].push_back(stroke);
    }

    for (auto &kv: strokesMap) {
        int color = kv.first;
        auto &strokes = kv.second;
        Rect r = strokes.front()->worldBbox();

        for (auto &s: strokes) {
            r.expand(s->worldBbox());
        }

        result[color] = r;
    }

    return result;
}

void Scene::reset( Stroke* s, bool purgeUnprotected )
{
  while ( purgeUnprotected && m_strokes.size() > m_protect ) {
    m_strokes[m_strokes.size()-1]->reset(m_world);
    m_strokes.pop_back();
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

  clearWithDelete(m_strokes);
  clearWithDelete(m_deletedStrokes);
  if ( m_world ) {
    //step is required to actually destroy bodies and joints
    m_world->Step( ITERATION_TIMESTEPf, SOLVER_ITERATIONS );
  }
  m_log.clear();
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

static std::list<std::string>
splitLines(const std::string &str)
{
    std::list<std::string> result;

    size_t pos = 0;
    do {
        size_t start = pos;
        pos = str.find('\n', pos + 1);
        size_t end = (pos == std::string::npos ? pos : pos - start);
        result.push_back(str.substr(start, end));
        pos++;
    } while (pos != std::string::npos + 1);

    return result;
}

class SceneSVGVisitor : public tinyxml2::XMLVisitor {
public:
    SceneSVGVisitor(Scene *scene)
        : tinyxml2::XMLVisitor()
        , scene(scene)
    {
        std::cout << "SVG Visitor" << std::endl;
    }

    virtual bool VisitEnter(const tinyxml2::XMLElement &element, const tinyxml2::XMLAttribute *firstAttribute) {
        if (strcmp(element.Name(), "np:meta") == 0) {
            const tinyxml2::XMLAttribute *attr;

            attr = element.FindAttribute("title");
            if (attr) {
                scene->m_title = attr->Value();
            }

            attr = element.FindAttribute("background");
            if (attr) {
                scene->m_bg = attr->Value();
            }

            attr = element.FindAttribute("author");
            if (attr) {
                scene->m_author = attr->Value();
            }

            attr = element.FindAttribute("gravity");
            if (attr) {
                scene->setGravity(attr->Value());
            }
        } else if (strcmp(element.Name(), "np:interaction") == 0) {
            const tinyxml2::XMLAttribute *color = element.FindAttribute("np:color");
            const tinyxml2::XMLAttribute *action = element.FindAttribute("np:action");

            if (color && action) {
                scene->m_interactions.add(color->Value(), action->Value());
            } else {
                std::cerr << "Invalid np:interaction" << std::endl;
            }
        } else if (strcmp(element.Name(), "path") == 0) {
            const tinyxml2::XMLAttribute *flags = element.FindAttribute("np:flags");
            const tinyxml2::XMLAttribute *color = element.FindAttribute("np:color");
            const tinyxml2::XMLAttribute *rgb = element.FindAttribute("stroke");
            const tinyxml2::XMLAttribute *data = element.FindAttribute("d");

            if (flags && color && rgb && data) {
                scene->m_strokes.push_back(new Stroke(flags->Value(),
                                                      rgb->Value(),
                                                      data->Value()));
            } else {
                std::cerr << "Invalid path" << std::endl;
            }
        } else if (strcmp(element.Name(), "np:event") == 0) {
            const tinyxml2::XMLAttribute *attr = element.FindAttribute("value");

            if (attr) {
                scene->m_log.push_back(std::string(attr->Value()));
            } else {
                std::cerr << "Invalid np:event" << std::endl;
            }
        } else {
            std::cerr << "Unhandled XML element: " << element.Name() << std::endl;
        }

        return true;
    }

private:
    Scene *scene;
};

bool Scene::load(const std::string &level)
{
    clear();
    resetWorld();
    m_dynamicGravity = false;
    m_step = 0;

    if (level.find("<svg") == 0) {
        tinyxml2::XMLDocument doc;
        doc.Parse(level.c_str());
        SceneSVGVisitor visitor(this);
        doc.Accept(&visitor);
    } else {
        // NPH format
        for (std::string &line: splitLines(level)) {
            std::string value = line.substr(line.find(':') + 1);
            switch (line[0]) {
                case 'T':
                    m_title = value;
                    break;
                case 'B':
                    m_bg = value;
                    break;
                case 'A':
                    m_author = value;
                    break;
                case 'S':
                    m_strokes.push_back(new Stroke(line));
                    break;
                case 'I':
                    m_interactions.parse(value);
                    break;
                case 'G':
                    setGravity(line);
                    break;
                case 'E':
                    m_log.push_back(value);
                    break;
                default:
                    printf("Unparsed: '%s'\n", line.c_str());
                    break;
            }
        }
    }

    protect();

    int events = m_log.size();
    if (events) {
        printf("loaded log=%d\n", events);
    }

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
    o << m_interactions.serialize();
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

