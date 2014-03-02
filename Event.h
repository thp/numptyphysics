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

#ifndef EVENT_H
#define EVENT_H
#include <SDL.h>

// custom SDL User Event code
const int WORKER_DONE = 1;

struct Event
{

  enum Code {
    NOP,
    DRAWBEGIN,
    DRAWMORE,
    DRAWEND,
    MOVEBEGIN,
    MOVEMORE,
    MOVEEND,
    SELECT,
    FOCUS,
    CANCEL,
    OPTION,
    CLOSE,
    DONE,
    QUIT,
    EDIT,
    MENU,
    DELETE,
    NEXT,
    PREVIOUS,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    RESET,
    UNDO,
    PAUSE,
    PLAY,
    REPLAY,
    SAVE,
    SEND,
    TEXT
  };

  enum Modes {
    MOD_BUTTON_LEFT = SDL_BUTTON(SDL_BUTTON_LEFT),
    MOD_BUTTON_RIGHT = SDL_BUTTON(SDL_BUTTON_RIGHT),
    MOD_BUTTON_MIDDLE = SDL_BUTTON(SDL_BUTTON_MIDDLE),
    MOD_CTRL = 16,
    MOD_SHIFT = 32
  };
  
  Code code;
  int  x,y;
  char c;
  char mods;

  Event(Code op=NOP, char cc=0) : code(op), c(cc), mods(g_mods) {}
  Event(Code op, int xx, int yy=0, char cc=0) : code(op), x(xx), y(yy), c(cc), mods(g_mods) {}
  Event(char cc) : code(TEXT), c(cc), mods(g_mods) {}

  static char g_mods;
};


struct EventMap
{
  virtual Event process(const SDL_Event& ev)=0;
};


class BasicEventMap : public EventMap
{
 public:
  struct KeyPair { int sym;/*SDLKey sym; */Event::Code ev; };
  struct ButtonPair { unsigned char button; Event::Code down; Event::Code move; Event::Code up; };
  BasicEventMap( const KeyPair* keys, const ButtonPair* buttons );
  Event process(const SDL_Event& ev);
 protected:
  //const KeyPair* lookupKey(SDLKey sym);
  const ButtonPair* lookupButton(unsigned char button);
 private:
  const KeyPair* m_keys;
  const ButtonPair* m_buttons;
};


enum EventMapType
{
  GAME_MAP,
  GAME_MOVE_MAP,
  GAME_ERASE_MAP,
  APP_MAP,
  EDIT_MAP,
  UI_BUTTON_MAP,
  UI_DRAGGABLE_MAP,
  UI_DIALOG_MAP,
};

#endif //EVENT_H
