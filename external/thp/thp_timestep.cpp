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

#include "thp_timestep.h"

thp::Timestep::Timestep(float fps)
    : fps(fps)
    , tick_last(0)
    , tick_accumulator(0)
    , paused_start(-1)
    , running_time(0)
{
}

void
thp::Timestep::update(long now, std::function<void()> callback)
{
    tick_accumulator += (now - tick_last);

    tick_last = now;

    long ticks = 1000 / fps;
    while (tick_accumulator > ticks) {
        callback();
        tick_accumulator -= ticks;
        running_time += ticks;
    }
}

void
thp::Timestep::paused()
{
}

void
thp::Timestep::resumed(long now)
{
    // Empty the accumulator, so we don't fast-forward
    tick_accumulator = 0;
    tick_last = now;
}
