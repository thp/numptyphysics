/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
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

#include "Stroke.h"
#include "Scene.h"

#include "thp_format.h"

#include <sstream>
#include <string>

static constexpr const int SVG_STROKE_WIDTH = 3;

static struct {
    const char *name;
    Attribute attribute;
} ATTRIBUTE_NAMES[] = {
    "token", ATTRIB_TOKEN,
    "goal", ATTRIB_GOAL,
    "fixed", ATTRIB_GROUND,
    "sleeping", ATTRIB_SLEEPING,
    "decor", ATTRIB_DECOR,
    "rope", ATTRIB_ROPE,
    "interactive", ATTRIB_INTERACTIVE,
};

Stroke::Stroke(const Path &path)
    : m_rawPath(path)
{
    m_colour = NP::Colour::DEFAULT;
    m_attributes = 0;
    m_origin = m_rawPath.point(0);
    m_rawPath.translate( -m_origin );
    reset();
}

Stroke::Stroke(const std::string &str)
{
    int col = 0;
    m_colour = NP::Colour::DEFAULT;
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
            case 'r': setAttribute( ATTRIB_ROPE ); break;
            case 'i': setAttribute( ATTRIB_INTERACTIVE ); break;
            default:
                      if ( *s >= '0' && *s <= '9' ) {
                          col = col*10 + *s -'0';
                      }
                      break;
        }
        s++;
    }
    if ( col >= 0 && col < NP::Colour::count ) {
        m_colour = NP::Colour::values[col];
    }
    if ( *s++ == ':' ) {
        m_rawPath = Path(s);
    }
    if ( m_rawPath.size() < 2 ) {
        throw "invalid stroke def";
    }
    m_origin = m_rawPath.point(0);
    m_rawPath.translate( -m_origin );
    setAttribute( ATTRIB_DUMMY );
}

Stroke::Stroke(const std::string &flags, const std::string &rgb, const std::string &svgpath)
{
    m_colour = NP::Colour::DEFAULT;
    m_attributes = 0;
    m_origin = Vec2(400,240);
    reset();

    int r = 0, g = 0, b = 0;
    if (sscanf(rgb.c_str(), "#%02x%02x%02x", &r, &g, &b) == 3) {
        m_colour = (r & 0xff) << 16 | (g & 0xff) << 8 | (b & 0xff);
    }

    std::map<std::string,Attribute> m;
    for (auto &e: ATTRIBUTE_NAMES) {
        m[e.name] = e.attribute;
    }

    std::istringstream iss(flags);
    std::string flag;
    while (getline(iss, flag, ' ')) {
        setAttribute(m[flag]);
    }

    m_rawPath = Path::fromSVG(svgpath);

    m_origin = m_rawPath.point(0);
    m_rawPath.translate(-m_origin);
    setAttribute(ATTRIB_DUMMY);
}

void
Stroke::reset(b2World *world)
{
    if (m_body && world) {
        world->DestroyBody(m_body);
    }

    m_body = NULL;
    m_xformAngle = 7.0f;
    m_jointed[0] = m_jointed[1] = false;
    m_shapePath = m_rawPath;
    m_hide = 0;
}

std::string
Stroke::asString()
{
    std::stringstream classes;
    for (auto &e: ATTRIBUTE_NAMES) {
        if (hasAttribute(e.attribute)) {
            classes << thp::format("%s ", e.name);
        }
    }

    std::stringstream points;
    Path opath = m_rawPath;
    opath.translate(m_origin);
    for (int i=0; i<opath.size(); i++) {
        const Vec2& p = opath.point(i);
        points << thp::format("%d %d", p.x, p.y);
        if (i < opath.size() - 1) {
            points << 'L';
        }
    }

    return thp::format("<path class=\"%s\" fill=\"none\" stroke=\"#%06x\" stroke-width=\"%d\" d=\"M%s\" />",
                       classes.str().c_str(), m_colour, SVG_STROKE_WIDTH, points.str().c_str());
}

void
Stroke::setAttribute(Attribute a)
{
    m_attributes |= a;
    if ( m_attributes & ATTRIB_TOKEN )     m_colour = NP::Colour::RED;
    else if ( m_attributes & ATTRIB_GOAL ) m_colour = NP::Colour::YELLOW;
}

void
Stroke::clearAttribute(Attribute a)
{
    m_attributes &= ~a;
}

bool
Stroke::hasAttribute(Attribute a)
{
    return (m_attributes&a) != 0;
}

void
Stroke::setColour(int c)
{
    m_colour = c;
}

void
Stroke::createBodies(b2World &world)
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

void
Stroke::determineJoints(Stroke *other, std::vector<Joint> &joints)
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
                joints.push_back( Joint(this,other,end) );
            }
        }
    }
}

void
Stroke::join(b2World *world, Stroke *other, unsigned char end)
{
    if ( !m_jointed[end] ) {
        b2Vec2 p = m_xformedPath.endpt( end );
        p *= 1.0f/PIXELS_PER_METREf;
        JointDef j( m_body, other->m_body, p );
        world->CreateJoint( &j );
        m_jointed[end] = true;
    }
}

bool
Stroke::maybeCreateJoint(b2World &world, Stroke *other)
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

void
Stroke::draw(Canvas &canvas, int a)
{
    if (m_hide >= HIDE_STEPS) {
        return;
    }

    transform();
    canvas.drawPath(m_screenPath, m_colour, a);

    if ( false /* drawJoints */ ) {
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

std::list<Stroke *>
Stroke::ropeify(Scene &scene)
{
    Path path = m_rawPath;
    path.simplify(SIMPLIFY_THRESHOLDf);
    // TODO: Split path up if its parts are too long

    int color = NP::Colour::toIndex(m_colour);
    int attr = m_attributes | ATTRIB_ROPE;

    std::list<Stroke *> result;
    for (int i=0; i<path.size()-1; i++) {
        Stroke *s = scene.newStroke(Path(path[i] + m_origin), color, attr);
        scene.extendStroke(s, path[i+1] + m_origin);
        result.push_back(s);
    }
    return result;
}

void
Stroke::addPoint(const Vec2 &pp)
{
    Vec2 p = pp; p -= m_origin;
    if ( p == m_rawPath.point( m_rawPath.numPoints()-1 ) ) {
    } else {
        m_rawPath.push_back( p );
    }
}

void
Stroke::origin(const Vec2 &p)
{
    // todo
    if ( m_body ) {
        b2Vec2 pw = p;
        pw *= 1.0f/PIXELS_PER_METREf;
        m_body->SetXForm( pw, m_body->GetAngle() );
    }
    m_origin = p;
}

b2Body *
Stroke::body()
{
    return m_body;
}

float32
Stroke::distanceTo(const Vec2 &pt)
{
    float32 best = 100000.0;
    transform();
    for ( int i=1; i<m_xformedPath.numPoints(); i++ ) {
        Segment s( m_xformedPath.point(i-1), m_xformedPath.point(i) );
        float32 d = s.distanceTo( pt );
        if ( d < best ) {
            best = d;
        }
    }
    return best;
}

Rect
Stroke::screenBbox()
{
    transform();
    return m_screenBbox;
}

Rect
Stroke::worldBbox()
{
    return m_xformedPath.bbox();
}

void
Stroke::hide()
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

bool
Stroke::hidden()
{
    return m_hide >= HIDE_STEPS;
}

int
Stroke::numPoints()
{
    return m_rawPath.numPoints();
}

const Vec2 &
Stroke::endpt(unsigned char end)
{
    return m_xformedPath.endpt(end);
}

void
Stroke::process()
{
    float32 thresh = SIMPLIFY_THRESHOLDf;
    m_rawPath.simplify( thresh );
    m_shapePath = m_rawPath;

    while ( m_shapePath.numPoints() > MULTI_VERTEX_LIMIT ) {
        thresh += SIMPLIFY_THRESHOLDf;
        m_shapePath.simplify( thresh );
    }
}

bool
Stroke::transform()
{
    // distinguish between xformed raw and shape path as needed
    if ( m_hide ) {
        if ( m_hide < HIDE_STEPS ) {
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
            b2Mat22 rot( m_body->GetAngle() );
            b2Vec2 orig = PIXELS_PER_METREf * m_body->GetPosition();
            m_xformedPath = m_rawPath;
            m_xformedPath.rotate( rot );
            m_xformedPath.translate( Vec2(orig) );
            m_xformAngle = m_body->GetAngle();
            m_xformPos = m_body->GetPosition();
            m_screenPath = m_xformedPath;
            m_screenBbox = m_screenPath.bbox();
        } else {
            return false;
        }
    } else {
        m_xformedPath = m_rawPath;
        m_xformedPath.translate( m_origin );
        m_screenPath = m_xformedPath;
        m_screenBbox = m_screenPath.bbox();
        return !hasAttribute(ATTRIB_DECOR);
    }
    return true;
}
