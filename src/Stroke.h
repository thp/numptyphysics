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

#ifndef NUMPTYPHYSICS_STROKE_H
#define NUMPTYPHYSICS_STROKE_H

#include "Common.h"
#include "Config.h"
#include "Canvas.h"
#include "Colour.h"

#include <vector>
#include <list>


class Stroke;
class Scene;

enum Attribute {
  ATTRIB_DUMMY = 0,
  ATTRIB_GROUND = 1,
  ATTRIB_TOKEN = 2,
  ATTRIB_GOAL = 4,
  ATTRIB_DECOR = 8,
  ATTRIB_SLEEPING = 16,
  ATTRIB_HIDDEN = 32,
  ATTRIB_DELETED = 64,
  ATTRIB_ROPE = 128,
  ATTRIB_INTERACTIVE = 256,

  ATTRIB_CLASSBITS = ATTRIB_TOKEN | ATTRIB_GOAL,
  ATTRIB_UNJOINABLE = ATTRIB_DECOR | ATTRIB_HIDDEN | ATTRIB_DELETED,
};

struct Joint {
    Joint(Stroke *joiner, Stroke *joinee, unsigned char end)
        : joiner(joiner)
        , joinee(joinee)
        , end(end)
    {
    }

    Stroke *joiner;
    Stroke *joinee;
    unsigned char end; // of joiner
};

struct JointDef : public b2RevoluteJointDef {
    JointDef(b2Body *b1, b2Body *b2, const b2Vec2 &pt)
    {
        Initialize(b1, b2, pt);
        maxMotorTorque = 10.0f;
        motorSpeed = 0.0f;
        enableMotor = true;
    }
};

struct BoxDef : public b2PolygonDef {
    float32 vec2Angle(b2Vec2 v) { return b2Atan2(v.y, v.x); }

    void init(const Vec2 &p1, const Vec2 &p2, int attr)
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
        if (attr & ATTRIB_GROUND) {
            density = 0.0f;
        } else if (attr & ATTRIB_GOAL) {
            density = 100.0f;
        } else if (attr & ATTRIB_TOKEN) {
            density = 3.0f;
            friction = 0.1f;
        } else {
            density = 5.0f;
        }
        restitution = 0.2f;
    }
};

class Stroke {
public:
    Stroke(const Path &path);
    Stroke(const std::string &str);

    void reset(b2World *world=nullptr);
    std::string asString();

    void setAttribute(Attribute a);
    void clearAttribute(Attribute a);
    bool hasAttribute(Attribute a);

    void setColour(int c);
    int colour() { return m_colour; }

    void createBodies(b2World &world);
    void determineJoints(Stroke *other, std::vector<Joint> &joints);
    void join(b2World *world, Stroke *other, unsigned char end);
    bool maybeCreateJoint(b2World &world, Stroke *other);
    void draw(Canvas &canvas, int a);
    std::list<Stroke *> ropeify(Scene &scene);

    void addPoint(const Vec2 &pp);
    void origin(const Vec2 &p);
    b2Body *body();

    float32 distanceTo(const Vec2 &pt);

    Rect screenBbox();
    Rect worldBbox();

    void hide();
    bool hidden();
    int numPoints();

    const Vec2 &endpt(unsigned char end);

    Vec2 origin() { return m_origin; }

private:
    void process();
    bool transform();

private:
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
    b2Body*   m_body;
    bool      m_jointed[2];
    int       m_hide;
};


#endif /* NUMPTYPHYSICS_STROKE_H */
