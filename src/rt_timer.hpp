//! Simple time controller implementation

#ifndef RT_TIMER_HPP_
#define RT_TIMER_HPP_

#include "rt_common.hpp"

namespace rt {
    /// Simple time manager implementation
    class timer {
    public:
        timer() = default;

        float get_delta_time() const noexcept {
            return delta_time;
        }

        float get_time() const noexcept {
            return time;
        }

        float get_fps() const noexcept {
            return fps;
        }

        bool is_fps_new() const noexcept {
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

        /// Get float-point duration in seconds between two time points
        static float time_between( clock::time_point start, clock::time_point end ) {
            return std::chrono::duration<float>(end - start).count();
        }

        clock::time_point start = clock::now();
        clock::time_point now = clock::now();

        float delta_time = 0.001f;
        float time = 0.001f;

        clock::duration fps_measure_duration = std::chrono::seconds(1);
        clock::time_point fps_last_measure = clock::now();
        std::uint32_t fps_frames_from_measure = 1;
        float fps = 0.0f;
    };
}

#endif // !defined(RT_TIMER_HPP_)

// rt_timer.hpp
