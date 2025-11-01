//! Simple time controller implementation

#ifndef RT_TIMER_HPP_
#define RT_TIMER_HPP_

#include "rt_common.hpp"

namespace rt {
    class timer {
    public:
        timer() {
            start = now = clock::now();
            delta_time = time = 0.001f;

            fps_measure_duration = std::chrono::seconds(1);
            fps_last_measure = now;
            fps_frames_from_measure = 1;
            fps = 0.0f;
        }

        float get_delta_time() const {
            return delta_time;
        }

        float get_time() const {
            return time;
        }

        float get_fps() const {
            return fps;
        }

        bool is_fps_new() const {
            return fps_frames_from_measure == 0;
        }

        void update() {
            clock::time_point new_now = clock::now();

            delta_time = time_between(now, new_now);
            time = time_between(start, new_now);
            now = new_now;

            fps_frames_from_measure++;
            if (now - fps_last_measure > fps_measure_duration) {
                fps = fps_frames_from_measure / time_between(fps_last_measure, now);
                fps_last_measure = now;
                fps_frames_from_measure = 0;
            }
        }

    private:
        using clock = std::chrono::steady_clock;

        static float time_between( clock::time_point start, clock::time_point end ) {
            return std::chrono::duration<float>(end - start).count();
        }

        clock::time_point start;
        clock::time_point now;

        float delta_time;
        float time;

        clock::duration fps_measure_duration;
        clock::time_point fps_last_measure;
        std::uint32_t fps_frames_from_measure;
        float fps;
    };
}

#endif // !defined(RT_TIMER_HPP_)

// rt_timer.hpp
