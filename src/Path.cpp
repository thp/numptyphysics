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

#include <cstring>
#include <iostream>

#include "Path.h"


static float32 calcDistanceToLine( const Vec2& pt,
				 const Vec2& l1, const Vec2& l2,
				 bool* withinLine=NULL )
{
  b2Vec2 l = l2 - l1; 
  b2Vec2 w = pt - l1;
  float32 mag = l.Normalize();
  float32 dist = b2Cross( w, l );
  if ( withinLine ) {
    float32 dot = b2Dot( l, w );
    *withinLine = ( dot >= 0.0f && dot <= mag );
  }  
  return b2Abs( dist );
}


static float32 calcDistance( const Vec2& l1, const Vec2& l2 ) 
{
  return b2Vec2(l1-l2).Length();
}


float32 Segment::distanceTo( const Vec2& p )
{
  bool withinLine;
  float32 d = calcDistanceToLine( p, m_p1, m_p2, &withinLine );
  if ( !(m_p1 == m_p2) && withinLine ) {
    return d;
  } else {
    return b2Min( calcDistance( p, m_p2 ), calcDistance( p, m_p1 ) );
  }
}


Path::Path()
    : std::vector<Vec2>()
{
}

Path::Path(const Vec2 &p)
    : std::vector<Vec2>()
{
    push_back(p);
}

Path::Path(int n, Vec2* p)
    : std::vector<Vec2>()
{
    for (int i=0; i<n; i++) {
        push_back(*p++);
    }
}

Path::Path( const char *s )
{
  float32 x,y;      
  while ( sscanf( s, "%f,%f", &x, &y )==2) {
    push_back( Vec2((int)x,(int)y) );
    while ( *s && *s!=' ' && *s!='\t' ) s++;
    while ( *s==' ' || *s=='\t' ) s++;
  }
}

class SVGPathTokenizer {
public:
    SVGPathTokenizer(const std::string &path);

    bool next(std::string &output);

private:
    std::string m_path;
    int m_ipos;
};

SVGPathTokenizer::SVGPathTokenizer(const std::string &path)
    : m_path(path)
    , m_ipos(0)
{
}

static bool _isnumber(char c)
{
    return ((c >= '0' && c <= '9') || c == '.' || c == '-');
}

static bool _isspace(char c)
{
    return (c == ' ' || c == ',');
}

static bool _isctrl(char c)
{
    return (c == 'm' || c == 'M' || c == 'L' || c == 'l');
}

bool
SVGPathTokenizer::next(std::string &output)
{
    if (m_ipos < m_path.length()) {
        // Scan forward to first non-space character
        while (m_ipos < m_path.length() && _isspace(m_path.at(m_ipos))) {
            m_ipos++;
        }

        int start = m_ipos;
        char c = m_path.at(start);
        if (_isctrl(c)) {
            output = m_path.substr(start, 1);
            m_ipos++;
            return true;
        }

        if (_isnumber(c)) {
            while (m_ipos < m_path.length() && _isnumber(m_path.at(m_ipos))) {
                m_ipos++;
            };

            int end = m_ipos;
            output = m_path.substr(start, end-start);
            m_ipos++;
            return true;
        }
    }

    return false;
}

class SVGPathParser {
public:
    SVGPathParser(const std::string &path);

    bool next(Vec2 &output);

private:
    SVGPathTokenizer m_tokenizer;
    Vec2 m_current;
    bool m_current_valid;
    enum Positioning {
        M_ABSOLUTE,
        M_RELATIVE,
    };
    enum Positioning m_positioning;
};

SVGPathParser::SVGPathParser(const std::string &path)
    : m_tokenizer(path)
    , m_current()
    , m_positioning(M_ABSOLUTE)
{
}

bool
SVGPathParser::next(Vec2 &output)
{
    std::string a, b;
    if (!m_tokenizer.next(a)) {
        return false;
    }

    if (a == "m" || a == "l" || a == "M" || a == "L") {
        if (a == "m" || a == "l") {
            m_positioning = M_RELATIVE;
        } else if (a == "M" || a == "L") {
            m_positioning = M_ABSOLUTE;
        }

        if (!m_tokenizer.next(a)) {
            std::cerr << "Warning: Incomplete coordinate after " << a << std::endl;
            return false;
        }
    }

    if (!m_tokenizer.next(b)) {
        std::cerr << "Warning: Incomplete coordinate after " << a << std::endl;
        return false;
    }

    Vec2 pos(atof(a.c_str()), atof(b.c_str()));
    if (m_current_valid && m_positioning == M_RELATIVE) {
        m_current += pos;
    } else {
        m_current = pos;
    }

    output = m_current;
    m_current_valid = true;
    return true;
}

Path
Path::fromSVG(const std::string &svgpath)
{
    Path path;

    SVGPathParser parser(svgpath);

    Vec2 pos;
    while (parser.next(pos)) {
        path.push_back(pos);
    }

    return path;
}

void Path::makeRelative() 
{
  for (int i=size()-1; i>=0; i--) 
    at(i)-=at(0); 
}


Path& Path::translate(const Vec2& xlate) 
{
  for (int i=0;i<size();i++)
    at(i) += xlate; 
  return *this;
}

Path& Path::rotate(const b2Mat22& rot) 
{
  float32 j1 = rot.col1.x;
  float32 k1 = rot.col1.y;
  float32 j2 = rot.col2.x;
  float32 k2 = rot.col2.y;
  Vec2 v;

  for (int i=0;i<size();i++) {
    //at(i) = b2Mul( rot, at(i) );
    at(i) = Vec2( j1 * at(i).x + j2 * at(i).y,
		  k1 * at(i).x + k2 * at(i).y );
  }
  return *this;
}

Path& Path::scale(float32 factor)
{
  for (int i=0;i<size();i++) {
    at(i).x = at(i).x * factor;
    at(i).y = at(i).y * factor;
  }
  return *this;
}

void Path::simplify( float32 threshold )
{
  bool keepflags[size()];
  memset( &keepflags[0], 0, sizeof(keepflags) );

  keepflags[0] = keepflags[size()-1] = true;
  simplifySub( 0, size()-1, threshold, &keepflags[0] );

  int k=0;
  for ( int i=0; i<size(); i++ ) {
    if ( keepflags[i] ) {
      at(k++) = at(i);
    }
  }
  resize(k);

  // remove duplicate points (shouldn't be any)
  std::vector<Vec2> result;
  Vec2 old = at(0);
  result.push_back(old);
  for (int i=1; i<size(); i++) {
      Vec2 cur = at(i);
      if (old != cur) {
          result.push_back(cur);
      }
      old = cur;
  }
  std::swap(*this, result);
}

void Path::simplifySub( int first, int last, float32 threshold, bool* keepflags )
{
  float32 furthestDist = threshold;
  int furthestIndex = 0;
  if ( last - first > 1 ) {
    Segment s( at(first), at(last) );
    for ( int i=first+1; i<last; i++ ) {
      float32 d = s.distanceTo( at(i) );
      if ( d > furthestDist ) {
	furthestDist = d;
	furthestIndex = i;
      }
    }
    if ( furthestIndex != 0 ) {
      keepflags[furthestIndex] = true;
      simplifySub( first, furthestIndex, threshold, keepflags );
      simplifySub( furthestIndex, last, threshold, keepflags );
    }
  }
}

Rect Path::bbox() const
{
    if (size() == 0) {
        return Rect(Vec2(), Vec2());
    }

    Rect r(at(0), at(0));
    for (auto &p: *this) {
        r.expand(p);
    }
    return r;
}
