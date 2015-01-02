/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2009 Tim Edmonds <numptyphysics@gmail.com>
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
#include "Event.h"

BasicEventMap::BasicEventMap( const KeyPair* keys, const ButtonPair* buttons )
  : m_keys(keys), m_buttons(buttons)
{}

Event BasicEventMap::process(const ToolkitEvent& ev)
{
  const ButtonPair *inf = 0;
  const KeyPair *key = 0;

  switch (ev.type) {
      case ToolkitEvent::PRESS:
          inf = lookupButton(ev.key);
          if (inf) {
              return Event(inf->down, ev.x, ev.y);
          }
          break;
      case ToolkitEvent::RELEASE:
          inf = lookupButton(ev.key);
          if (inf) {
              return Event(inf->up, ev.x, ev.y);
          }
          break;
      case ToolkitEvent::MOVE:
          inf = lookupButton(ev.key);
          if (inf) {
              return Event(inf->move, ev.x, ev.y);
          }
          break;
      case ToolkitEvent::KEYDOWN:
          key = lookupKey(ev.key);
          if (key) {
              return Event(key->ev, (char)ev.key);
          }
          break;
      default:
          break;
  }

  return Event();
}

const BasicEventMap::KeyPair*
BasicEventMap::lookupKey(int key)
{
  const KeyPair* p = m_keys;
  while (p && p->sym) {
      if (p->sym == key) {
          return p;
      }
      p++;
  }
  return NULL;
}

const BasicEventMap::ButtonPair*
BasicEventMap::lookupButton(unsigned char button)
{
    const ButtonPair* p = m_buttons;
    while (p && p->button) {
        if (p->button == button) {
            return p;
        }
        p++;
    }
    return NULL;
}


