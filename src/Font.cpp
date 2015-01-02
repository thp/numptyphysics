/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2009, 2010 Tim Edmonds <numptyphysics@gmail.com>
 * Coyright (c) 2014 Thomas Perl <m@thp.io>
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
#include "Font.h"
#include "Canvas.h"
#include "Config.h"


Font::Font( const std::string& file, int ptsize )
{
    std::string fname = Config::findFile(file);
    m_font = OS->renderer()->load(fname.c_str(), ptsize);
    m_height = metrics("M").y;
}


Vec2 Font::metrics( const std::string& text ) const
{
    Vec2 m;
    OS->renderer()->metrics(m_font, text.c_str(), &m.x, &m.y);
    return m;
}

void Font::drawLeft( Canvas* canvas, Vec2 pt,
		     const std::string& text, int colour ) const
{
    Image temp(m_font, text.c_str(), colour);
    canvas->drawImage(temp, pt.x, pt.y);
}

void Font::drawRight( Canvas* canvas, Vec2 pt,
		     const std::string& text, int colour ) const
{
  drawLeft( canvas, pt - Vec2(metrics(text).x,0), text, colour );
}

void Font::drawCenter( Canvas* canvas, Vec2 pt,
		       const std::string& text, int colour ) const
{
  drawLeft( canvas, pt - metrics(text)/2, text, colour );
}

void Font::drawWrap( Canvas* canvas, Rect area,
		     const std::string& text, int colour ) const
{
  Vec2 pos = area.tl;
  size_t i = 0, e=0;
  while ( i < text.length() ) {
    e = text.find_first_of(" \n\r\t",i);
    if ( i == e ) {
      i++;
    } else {
      std::string word = text.substr(i,i+e);
      Vec2 wm = metrics( word );
      if ( pos.x + wm.x > area.br.x ) {
	pos.x = area.tl.x;	
	pos.y += wm.y;	
      }
      drawLeft( canvas, pos, word, colour );
      i = e + 1;
    }
  }
  drawLeft( canvas, pos, text.substr(i), colour );
}


const Font* Font::titleFont()
{
  static Font* f = 0;
  if (!f) {
      f = new Font("femkeklaver.ttf",48);
  }
  return f;
}

const Font* Font::headingFont()
{
  static Font* f = 0;
  if (!f) {
      f = new Font("femkeklaver.ttf",32);
  }
  return f;
}

const Font* Font::blurbFont()
{
  static Font* f = 0;
  if (!f) {
      f = new Font("femkeklaver.ttf",24);
  }
  return f;
}

