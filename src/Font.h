/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2008, 2009, 2010 Tim Edmonds <numptyphysics@gmail.com>
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

#ifndef FONT_H
#define FONT_H

#include "Common.h"
#include "Path.h"
#include "Renderer.h"
#include <string>

class Canvas;

class Font
{
 public:
  Font( const std::string& file, int ptsize=10 );
  int height() const { return m_height; }
  Vec2 metrics( const std::string& text ) const;
  void drawLeft( Canvas* canvas, Vec2 pt,
		 const std::string& text, int colour ) const;
  void drawRight( Canvas* canvas, Vec2 pt,
		  const std::string& text, int colour ) const;
  void drawCenter( Canvas* canvas, Vec2 pt,
		   const std::string& text, int colour ) const;
  void drawWrap( Canvas* canvas, Rect area,
		 const std::string& text, int colour ) const;

  static const Font* titleFont();
  static const Font* headingFont();
  static const Font* blurbFont();
 private:
  NP::Font m_font;
  int m_height;
};


#endif //FONT_H
