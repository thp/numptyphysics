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

#ifndef COMMON_H
#define COMMON_H

#include "Box2D.h"
#include <vector>

#define ARRAY_SIZE(aRR) (sizeof(aRR)/sizeof((aRR)[0]))
#define ASSERT(a)

#define FIXED_SHIFT 16
#define FIXED_ONE (1<<FIXED_SHIFT)
#define FLOAT32_TO_FIXED(float32) ((int)(float32*(float32)FIXED_ONE))
#define FIXED_TO_INT(iNT) ((iNT)>>FIXED_SHIFT)

struct Vec2 {
  Vec2() : x(0), y(0) {}
  Vec2( const Vec2& o ) : x(o.x), y(o.y) {}
  explicit Vec2( const b2Vec2& o ) : x((int)o.x), y((int)o.y) {}
  Vec2( int xx, int yy ) : x(xx), y(yy) {}
  void operator+=( const Vec2& o ) { x+=o.x; y+=o.y; }
  void operator-=( const Vec2& o ) { x-=o.x; y-=o.y; }
  Vec2 operator-() { return Vec2(-x,-y); }
  void operator*=( int o ) { x*=o; y*=o; }
  bool operator==( const Vec2& o ) const { return x==o.x && y==o.y; }
  bool operator!=( const Vec2& o ) const { return !(*this==o); }
  operator b2Vec2() const { return b2Vec2((float32)x,(float32)y); } 
  Vec2 operator+( const Vec2& b ) const { return Vec2(x+b.x,y+b.y); }
  Vec2 operator-( const Vec2& b ) const { return Vec2(x-b.x,y-b.y); }
  Vec2 operator/( int r ) const { return Vec2(x/r,y/r); }
  Vec2 operator*( int r ) const { return Vec2(x*r,y*r); }
  int x,y;
};

template <typename T> inline T Min( T a, T b )
{
  return a < b ? a : b;
}

inline Vec2 Min( const Vec2& a, const Vec2& b )
{
  Vec2 r;
  r.x = Min(a.x,b.x);
  r.y = Min(a.y,b.y);
  return r;
}

template <typename T> inline T Max( T a, T b )
{
  return a >= b ? a : b;
}

inline Vec2 Max( const Vec2& a, const Vec2& b )
{
  Vec2 r;
  r.x = Max(a.x,b.x);
  r.y = Max(a.y,b.y);
  return r;
}

template <typename T>
inline int indexOf(const std::vector<T> &collection, T &o)
{
    int i = 0;

    for (auto &v: collection) {
        if (v == o) {
            return i;
        }

        i++;
    }

    return -1;
}

template <typename T>
inline void clearWithDelete(std::vector<T> &collection)
{
    for (auto &item: collection) {
        delete item;
    }
    collection.clear();
}

#define Sgn(a) ((a)<0?-1:1)
#define Abs(a) ((a)<0?-(a):(a))



struct Rect {
  Rect() {}
  Rect(bool) { clear(); }
  Rect( const Vec2& atl, const Vec2& abr ) : tl(atl), br(abr) {} 
  Rect( int x1, int y1, int x2, int y2 ) : tl(x1,y1), br(x2,y2) {}
  int width() const { return br.x-tl.x+1;  }
  int height() const { return br.y-tl.y+1;  }
  Vec2 size() const { return br-tl; }
  void clear() { tl.x=tl.y=br.x=br.y=0; }
  bool isEmpty() const { return tl.x==0 && br.x==0; }
  void grow(int by) { 
    if (!isEmpty()) {
      tl.x -= by; tl.y -= by;
      br.x += by; br.y += by;
    }
  }
  Rect shrunk(int by) {
      Rect o = *this;
      o.grow(-by);
      return o;
  }
  void expand( const Vec2& v ) { tl=Min(tl,v); br=Max(br,v); }
  void expand( const Rect& r ) { 
    if (isEmpty()) {
      *this = r;
    } else if (!r.isEmpty()) {
      expand(r.tl); 
      expand(r.br); 
    }
  }
  void clipTo( const Rect& r ) { tl=Max(tl,r.tl); br=Min(br,r.br); }
  bool contains( const Vec2& p ) const {
    return p.x >= tl.x && p.x <= br.x && p.y >= tl.y && p.y <= br.y;
  }
  bool contains( const Rect& p ) const {
    return contains(p.tl) && contains(p.br);
  }
  bool intersects( const Rect& r ) const {
    return r.tl.x <= br.x
      && r.tl.y <= br.y
      && r.br.x >= tl.x 
      && r.br.y >= tl.y;
  }
  Vec2 centroid() const { return (tl+br)/2; }
  Rect operator+( const Vec2& b ) const {
    Rect r=*this;
    r.tl += b; r.br += b;
    return r;
  }
  Vec2 tl, br;
  Vec2 tr() const { return Vec2(br.x, tl.y); }
  Vec2 bl() const { return Vec2(tl.x, br.y); }
};


#endif //COMMON_H
