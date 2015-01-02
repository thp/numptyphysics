/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
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

#ifndef THP_TIMESTEP_H
#define THP_TIMESTEP_H

#include <functional>

namespace thp {

class Timestep {
    public:
        Timestep(float fps);
        void update(long now, std::function<void()> callback);

        // Number of milliseconds in a tick
        int ms_per_tick() { return int(1000.f / fps); }

        // 0..1: Partial step between current and next step right now
        float partial() { return (float)tick_accumulator * fps / 1000.f; }

        // Pause and resume support
        void paused();
        void resumed(long now);
        bool is_paused() { return paused_start != -1; }

        // Running time without pauses
        long runtime() { return running_time + tick_accumulator; }

    private:
        float fps;
        long tick_last;
        long tick_accumulator;
        long paused_start;
        long running_time;
};

}; /* namespace thp */

#endif /* THP_TIMESTEP_H */
