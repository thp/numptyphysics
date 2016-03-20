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

#include "Script.h"
#include "Scene.h"

#include <string.h>

#include "petals_log.h"
#include "thp_format.h"


std::string
ScriptLogEntry::serialize(const ScriptLogEntry &e)
{
    return thp::format("@%d:%s:%d,%d:%d:%d", e.tick, e.ev.meta()->name, e.ev.pos.x, e.ev.pos.y, e.ev.userdata1, e.ev.userdata2);
}

static int
number(const std::string &s)
{
    // TODO: Error checking
    return atoi(s.c_str());
}

static std::vector<std::string>
split_by(const char *splits, const std::string &s)
{
    std::vector<std::string> result;

    int len = strlen(splits);
    size_t pos = 0;
    for (int i=0; i<len; i++) {
        size_t start = s.find(splits[i], pos);
        if (start == std::string::npos) {
            break;
        }
        start++;
        size_t end = len;
        if (i < len-1) {
            end = s.find(splits[i+1], start);
            if (end == std::string::npos) {
                break;
            }
        }
        result.push_back(s.substr(start, end-start));
        pos = end;
    }

    return result;
}

ScriptLogEntry
ScriptLogEntry::deserialize(const std::string &s)
{
    // @0:BEGIN_CREATE_STROKE_AT:605,243:2:0
    // ^ ^                      ^   ^   ^ ^
    auto matchobj = split_by("@::,::", s);

    if (matchobj.size() != 6) {
        LOG_FATAL("Cannot deserialize ScriptLogEntry: '%s'", s.c_str());
    }

    int tick = number(matchobj[0]);
    std::string operation = matchobj[1];
    Vec2 pos(number(matchobj[2]), number(matchobj[3]));
    int userdata1 = number(matchobj[4]);
    int userdata2 = number(matchobj[5]);

    return ScriptLogEntry(tick, SceneEvent(operation, pos, userdata1, userdata2));
}

void
ScriptHandler::start(ScriptLog *log)
{
    m_log = log;
    m_running = true;
    m_ticks = 0;
    m_index = 0;
}

void
ScriptHandler::stop()
{
    m_log = nullptr;
    m_running = false;
    m_ticks = 0;
    m_index = 0;
}

void
ScriptHandler::tick(Scene *scene)
{
    if (m_running) {
        m_ticks++;
    }
}

void
ScriptRecorder::onSceneEvent(const SceneEvent &ev)
{
    if (m_running) {
        if (m_log) {
            m_log->push_back(ScriptLogEntry(m_ticks, ev));
            m_index++;
        } else {
            LOG_WARNING("No log in ScriptRecorder onSceneEvent");
        }
    }
}

void
ScriptPlayer::tick(Scene *scene)
{
    ScriptHandler::tick(scene);

    while (m_running && m_index < m_log->size() && m_log->at(m_index).tick <= m_ticks) {
        scene->onSceneEvent(m_log->at(m_index).ev);
        m_index++;
    }
}
