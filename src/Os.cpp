/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2009, 2010 Tim Edmonds <numptyphysics@gmail.com>
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

#include "Os.h"

#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#ifdef __WIN32__
#include <windows.h>
#undef DELETE
#endif

#include "thp_format.h"
#include "petals_log.h"

OsObj OS;

static const BasicEventMap::KeyPair game_keymap[] = {
  { NUMPTYKEY_SPACE,    Event::PAUSE },
  { NUMPTYKEY_RETURN,   Event::PAUSE },
  { NUMPTYKEY_ESCAPE,   Event::UNDO  },
  { NUMPTYKEY_BACKSPACE,Event::UNDO  },
  { NUMPTYKEY_u,        Event::UNDO  },
  { NUMPTYKEY_DOWN,     Event::UNDO  },
  { NUMPTYKEY_F7,       Event::UNDO  },
  { NUMPTYKEY_s,        Event::SAVE  },
  { NUMPTYKEY_F4,       Event::OPTION},
  { NUMPTYKEY_m,        Event::MENU},
  { NUMPTYKEY_e,        Event::EDIT  },
  { NUMPTYKEY_F6,       Event::EDIT  },
  { NUMPTYKEY_r,        Event::RESET },
  { NUMPTYKEY_UP,       Event::RESET },
  { NUMPTYKEY_n,        Event::NEXT  },
  { NUMPTYKEY_RIGHT,    Event::NEXT  },
  { NUMPTYKEY_p,        Event::PREVIOUS },
  { NUMPTYKEY_LEFT,     Event::PREVIOUS },
  { NUMPTYKEY_v,        Event::REPLAY},
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

static const BasicEventMap::ButtonPair game_jetstream_mousemap[] = {
  { 1, Event::JETSTREAMBEGIN, Event::JETSTREAMMORE, Event::JETSTREAMEND },
  {}
};

static const BasicEventMap::KeyPair app_keymap[] = {
  { NUMPTYKEY_q, Event::QUIT },
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
  { NUMPTYKEY_UP,       Event::UP },
  { NUMPTYKEY_DOWN,     Event::DOWN },
  { NUMPTYKEY_RIGHT,    Event::RIGHT  },
  { NUMPTYKEY_LEFT,     Event::LEFT },
  {}
};

static const BasicEventMap::KeyPair ui_dialog_keymap[] = {
  { NUMPTYKEY_ESCAPE,   Event::CLOSE  },
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
  static BasicEventMap gameJetstreamMap(game_keymap,game_jetstream_mousemap);
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
  case GAME_JETSTREAM_MAP:
    return &gameJetstreamMap;
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
#ifdef __WIN32__
	if (mkdir(path.c_str()) != 0 ) {
#else
        if (mkdir(path.c_str(), 0755) != 0) {
#endif
            LOG_WARNING("Failed to create dir: %s", path.c_str());
            return false;
        } else {
            LOG_DEBUG("Created dir %s", path.c_str());
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

static Os *
g_os = nullptr;

const char
#ifdef __WIN32__
Os::pathSep = '\\';
#else
Os::pathSep = '/';
#endif

Os::Os()
{
    if (g_os != nullptr) {
        LOG_FATAL("OS instance already exists!");
    }

    g_os = this;
}

Os *
Os::get()
{
    return g_os;
}

static std::string
g_appDir;

static std::string
g_appName;

static long
log_get_ticks()
{
    return OS->ticks();
}

void
Os::init(int argc, char **argv)
{
    PetalsLog::init(log_get_ticks, thp::format);

    char buf[PATH_MAX];

    const char *progname = argv[0];
    const char *slash = strrchr(progname, Os::pathSep);

    if (!slash) {
#ifdef __WIN32__
	if (GetModuleFileName(NULL, buf, sizeof(buf)) != -1) {
#else
        if (readlink("/proc/self/exe", buf, sizeof(buf)) != -1) {
#endif
            progname = buf;
            slash = strrchr(progname, Os::pathSep);
        }
    }

    if (!slash) {
        throw "Could not determine application path";
    }

    g_appName = slash + 1;
    g_appDir = std::string(progname, slash-progname);
}

std::string
Os::appName()
{
    return g_appName;
}

std::string
Os::globalDataDir()
{
    std::string sourceData = thp::format("%s/data", g_appDir.c_str());

    // Prefer './data' in the source checkout if available
    if (exists(sourceData)) {
        return sourceData;
    }

    // System-wide installation
    return thp::format("%s/../share/%s/data", g_appDir.c_str(), appName().c_str());
}
