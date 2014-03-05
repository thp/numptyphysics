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
