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
#include "Font.h"
#include "Canvas.h"
#include "Config.h"
#include <SDL_ttf.h>

#define FONT(fONTpTR) ((TTF_Font*)((fONTpTR)->m_state))

struct FontCanvas : public Image
{
  FontCanvas( SDL_Surface* s )
    : Image( s )
  {}
};


Font::Font( const std::string& file, int ptsize )
{
  TTF_Init();
  std::string fname = Config::findFile(file);
  m_state = TTF_OpenFont( fname.c_str(), ptsize );
  m_height = metrics("M").y;
}


Vec2 Font::metrics( const std::string& text ) const
{
  Vec2 m;
  TTF_SizeText( FONT(this), text.c_str(), &m.x, &m.y );
  return m;
}


void Font::drawLeft( Canvas* canvas, Vec2 pt,
		     const std::string& text, int colour ) const
{
  SDL_Color fg = { colour>>16, colour>>8, colour };
  FontCanvas temp( TTF_RenderText_Blended( FONT(this),
					   text.c_str(),
					   fg ) );
  canvas->drawImage( &temp, pt.x, pt.y );
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
  static Font* f = new Font("femkeklaver.ttf",48);
  return f;
}

const Font* Font::headingFont()
{
  static Font* f = new Font("femkeklaver.ttf",32);
  return f;
}

const Font* Font::blurbFont()
{
  static Font* f = new Font("femkeklaver.ttf",24);
  return f;
}

