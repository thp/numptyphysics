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
