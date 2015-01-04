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
#ifndef SCRIPT_H
#define SCRIPT_H

#include "SceneEvent.h"

#include <iostream>
#include <vector>
#include <utility>

class Scene;

struct ScriptLogEntry {
    ScriptLogEntry(int tick, const SceneEvent &ev) : tick(tick), ev(ev) {}

    static std::string serialize(const ScriptLogEntry &e);
    static ScriptLogEntry deserialize(const std::string &s);

    int tick;
    SceneEvent ev;
};

class ScriptLog : public std::vector<ScriptLogEntry> {
};

class ScriptHandler {
public:
    ScriptHandler()
        : m_log(nullptr)
        , m_running(false)
        , m_ticks(0)
        , m_index(0)
    {}

    virtual void start(ScriptLog *log);
    virtual void stop();
    virtual void tick(Scene *scene);

    bool running() { return m_running; }
    int index() { return m_index; }

protected:
    ScriptLog *m_log;
    bool m_running;
    int m_ticks;
    int m_index;
};

class ScriptRecorder : public ScriptHandler {
public:
    void onSceneEvent(const SceneEvent &ev);
};

class ScriptPlayer : public ScriptHandler {
public:
    virtual void tick(Scene *scene);
};

#endif //SCRIPT_H
