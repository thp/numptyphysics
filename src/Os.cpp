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

#include "Os.h"
#include <SDL.h>

#include <sys/stat.h>
#include <unistd.h>

OsObj OS;

static const BasicEventMap::KeyPair game_keymap[] = {
  { SDLK_SPACE,    Event::PAUSE },
  { SDLK_KP_ENTER, Event::PAUSE },
  { SDLK_RETURN,   Event::PAUSE },
  { SDLK_ESCAPE,   Event::UNDO  },
  { SDLK_BACKSPACE,Event::UNDO  },
  { SDLK_u,        Event::UNDO  },
  { SDLK_DOWN,     Event::UNDO  },
  { SDLK_F7,       Event::UNDO  },
  { SDLK_s,        Event::SAVE  },
  { SDLK_F4,       Event::OPTION},
  { SDLK_m,        Event::MENU},
  { SDLK_e,        Event::EDIT  },
  { SDLK_F6,       Event::EDIT  },
  { SDLK_r,        Event::RESET },
  { SDLK_UP,       Event::RESET },
  { SDLK_n,        Event::NEXT  },
  { SDLK_RIGHT,    Event::NEXT  },
  { SDLK_p,        Event::PREVIOUS },
  { SDLK_LEFT,     Event::PREVIOUS },
  { SDLK_v,        Event::REPLAY},
  {}
};

static const BasicEventMap::ButtonPair game_mousemap[] = {
  { 1, Event::DRAWBEGIN, Event::DRAWMORE, Event::DRAWEND },
  {}
};

static const BasicEventMap::ButtonPair game_move_mousemap[] = {
  { 1, Event::MOVEBEGIN, Event::MOVEMORE, Event::MOVEEND },
  {}
};

static const BasicEventMap::ButtonPair game_erase_mousemap[] = {
  { 1, Event::NOP, Event::NOP, Event::DELETE },
  {}
};

static const BasicEventMap::KeyPair app_keymap[] = {
  { SDLK_q, Event::QUIT },
  {}
};


static const BasicEventMap::ButtonPair edit_mousemap[] = {
  { 1, Event::DRAWBEGIN, Event::DRAWMORE, Event::DRAWEND },
  { 2, Event::MOVEBEGIN, Event::MOVEMORE, Event::MOVEEND },
  { 3, Event::DELETE },
  {}
};

static const BasicEventMap::ButtonPair ui_button_mousemap[] = {
  { 1, Event::FOCUS, Event::FOCUS, Event::SELECT },
  {}
};

static const BasicEventMap::ButtonPair ui_draggable_mousemap[] = {
  { 1, Event::MOVEBEGIN, Event::MOVEMORE, Event::MOVEEND },
  {}
};

static const BasicEventMap::KeyPair ui_draggable_keymap[] = {
  { SDLK_UP,       Event::UP },
  { SDLK_DOWN,     Event::DOWN },
  { SDLK_RIGHT,    Event::RIGHT  },
  { SDLK_LEFT,     Event::LEFT },
  {}
};

static const BasicEventMap::KeyPair ui_dialog_keymap[] = {
  { SDLK_ESCAPE,   Event::CLOSE  },
  {}
};


class AppMap : public BasicEventMap
{
public:
  AppMap() : BasicEventMap( app_keymap, NULL ) {}
  Event process(const ToolkitEvent& ev)
  {
      if (ev.type == ToolkitEvent::QUIT) {
          return Event(Event::QUIT);
      } else {
          return BasicEventMap::process(ev);
      }
  }

};



EventMap* Os::getEventMap( EventMapType type )
{
  static BasicEventMap gameMap(game_keymap,game_mousemap);
  static BasicEventMap gameMoveMap(game_keymap,game_move_mousemap);
  static BasicEventMap gameEraseMap(game_keymap,game_erase_mousemap);
  static BasicEventMap editMap(NULL,edit_mousemap);
  static BasicEventMap uiButtonMap(NULL,ui_button_mousemap);
  static BasicEventMap uiDraggableMap(ui_draggable_keymap,ui_draggable_mousemap);
  static BasicEventMap uiDialogMap(ui_dialog_keymap,NULL);
  static AppMap appMap;

  switch (type) {
  case GAME_MAP:
    return &gameMap;
  case GAME_MOVE_MAP:
    return &gameMoveMap;
  case GAME_ERASE_MAP:
    return &gameEraseMap;
  case APP_MAP:
    return &appMap;
  case EDIT_MAP:
    return &editMap;
  case UI_BUTTON_MAP:
    return &uiButtonMap;
  case UI_DRAGGABLE_MAP:
    return &uiDraggableMap;
  case UI_DIALOG_MAP:
    return &uiDialogMap;
  }
  return NULL;
}


bool Os::ensurePath(const std::string& path)
{
    if (!exists(path)) {
        size_t sep = path.rfind(Os::pathSep);
        if ( sep != std::string::npos && sep > 0 ) {
            ensurePath(path.substr(0,sep));
        }
        if ( mkdir( path.c_str(), 0755)!=0 ) {
            fprintf(stderr,"failed to create dir %s\n", path.c_str());
            return false;
        } else {
            fprintf(stderr,"created dir %s\n", path.c_str());
            return true;
        }
    }
    return true;
}


bool Os::exists(const std::string& file)
{
    struct stat st;
    return stat(file.c_str(),&st) == 0;
}
