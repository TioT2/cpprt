//! Ray tracing engine definition file file

#ifndef RT_ENGINE_HPP_
#define RT_ENGINE_HPP_

#include "rt_shape.hpp"
#include "rt_random.hpp"

namespace rt {

    /// 'Mutex' synchronization primitive
    class spinlock {
    public:

        /// Lock
        void lock() {
            for (;;) {
                if (!lock_.exchange(true, std::memory_order_acquire))
                    break;

                while (lock_.load(std::memory_order_relaxed))
                    pause(); // something like _mm_pause
            }
        }

        /// Unlock
        void unlock() {
            lock_.store(false, std::memory_order_release);
        }

    private:

        /// Function that does nothing
        static void pause() {
            static volatile unsigned int counter = 0;
            counter += 4;
            counter += 4;
            counter += 4;
            counter += 4;
        }

        /// Spinlock state
        std::atomic_bool lock_ = false;
    };

    /// Parallel execution handler
    class executor {
    public:

        /// Construct task executor
        executor(
            std::size_t task_count,
            std::vector<std::function<void(std::size_t)>> thread_functions,
            std::uint64_t task_random_seed = 47
        ) {
            // Construct task vector
            tasks.reserve(task_count);
            for (std::size_t i = 0; i < task_count; i++)
                tasks.push_back(i);

            /// Randomize rendering order
            random::xoshiro256pp random {task_random_seed};
            for (std::size_t i = 0; i < tasks.size(); i++)
                std::swap(
                    tasks[random.next() % tasks.size()],
                    tasks[random.next() % tasks.size()]
                );

            /// Spawn rendering threads
            threads.reserve(thread_functions.size());
            for (auto fn : thread_functions)
                threads.push_back(std::thread([this, fn]() {
                    while (do_continue.load(std::memory_order_relaxed))
                        fn(tasks[task_index.fetch_add(1, std::memory_order_relaxed) % tasks.size()]);
                }));
        }

        /// Executor destructor
        ~executor() {
            do_continue.store(false, std::memory_order_relaxed);
            /// Join all threads
            for (auto &thr : threads)
                thr.join();
        }

    private:

        /// Current task index
        std::atomic_uint32_t task_index = 0;

        /// Continue if true (used to stop worker thread execution on destruction)
        std::atomic_bool do_continue = true;

        /// Array of tasks
        std::vector<std::uint32_t> tasks = {};

        /// Array of threads
        std::vector<std::thread> threads = {};
    };

    /// Camera location descriptor
    class camera {
    public:

        /// Construct camera from location-at-up triple
        static camera from_loc_dir_up(vec3 location, vec3 forward, vec3 approx_up) {
            vec3 right = forward.cross(approx_up).normalized();
            vec3 up = right.cross(forward).normalized();

            return camera {
                .location = location,
                .forward = forward,
                .right = right,
                .up = up,
            };
        }

        /// Camera origin
        vec3 location {0.0f, 0.0f, 4.0f};

        /// Forward direction
        vec3 forward {0.0f, 0.0f, -1.0f};

        /// Right direction
        vec3 right {1.0f, 0.0f, 0.0f};

        /// Up direction
        vec3 up {0.0f, 1.0f, 0.0f};
    };

    /// Single rendering 'task'
    class render_row {
    public:
        /// Construct single rendering row
        render_row(std::size_t width):
            source(std::make_unique<vec3[]>(width)),
            destination(std::make_unique<vec3[]>(width))
        {
        }

        /// Move constructor
        render_row(render_row &&other):
            collected_count(other.collected_count),
            frame_revision(other.frame_revision),
            source(std::move(other.source)),
            destination(std::move(other.destination))
        {
        }

        /// Source data lock (for collected_count, revision, source variable contents)
        spinlock source_lock;

        /// Destination data lock (not allow two threads to access single row at once)
        spinlock destination_lock;

        /// Count of collected frames
        std::uint32_t collected_count = 1;

        /// 'hash' of current renderer state, used for non-blocking camera movement
        std::uint32_t frame_revision = 0;

        /// Source pointer
        std::unique_ptr<vec3[]> source;

        /// Destination pointer
        std::unique_ptr<vec3[]> destination;
    };

    /// Ray-tracing engine
    class engine {
    public:

        /// Default sky tracing function
        static vec3 default_sky_trace( vec3 _dir ) {
            return vec3(0.30f, 0.47f, 0.80f);
        }

        /// engine constructor
        engine(
            std::unique_ptr<shape::shape> object,
            std::function<vec3(vec3)> trace_sky = default_sky_trace
        ) :
            object(std::move(object)),
            sky_trace_function(std::move(trace_sky))
        {
            set_render_resolution(160, 100);
            start_rendering();
        }

        /// Set size of target buffer
        void set_render_resolution( std::size_t width, std::size_t height ) {
            stop_rendering();

            render_width = width;
            render_height = height;

            rows.clear();
            for (std::size_t y = 0; y < height; y++)
                rows.push_back(render_row(width));

            start_rendering();
        }

        /// Set new camera state
        void set_camera(camera new_camera) {
            auto curr = dynamic_state;

            // Update dynamic state
            dynamic_state = std::make_shared<dynamic_frame_state>(dynamic_frame_state {
                .render_camera = new_camera,
                .revision = curr->revision + 1,
            });
        }

        /// Display frame
        void display_frame( std::byte *frame_ptr, std::size_t pitch ) {

            for (auto &row : rows) {
                std::lock_guard row_source_lock {row.source_lock};

                const float color_coef = 255.0f / row.collected_count;

                vec3 *data = row.source.get();
                std::size_t counter = render_width;
                while (counter--) {
                    std::uint32_t compressed = 0
                        | (static_cast<std::uint8_t>(data->x * color_coef) << 16)
                        | (static_cast<std::uint8_t>(data->y * color_coef) <<  8)
                        | (static_cast<std::uint8_t>(data->z * color_coef) <<  0);

                    std::memcpy(frame_ptr, &compressed, 4);
                    frame_ptr += 4;
                    data++;
                }
                frame_ptr += pitch - render_width * 4;
            }
        }

    private:

        /// Run executor of rendering process
        void start_rendering() {
            if (render_executor.has_value())
                return;

            std::size_t thread_count = std::thread::hardware_concurrency() - 1;
            std::vector<std::function<void(std::size_t)>> thread_fns;
            thread_fns.reserve(thread_count);

            for (std::size_t thread_index = 0; thread_index < thread_count; thread_index++) {
                random::xoshiro256pp thread_random{thread_index};
                const std::size_t width = render_width;
                const std::size_t height = render_height;

                float x_scale = 1.0f;
                float y_scale = 1.0f;
                if (width > height)
                    x_scale = (float)width / height;
                else
                    y_scale = (float)height / width;

                const float x_mul = 2.0f * x_scale / width;
                const float y_mul = 2.0f * y_scale / height;

                thread_fns.push_back([
                    thread_random,
                    width,
                    height,
                    x_mul,
                    y_mul,
                    x_scale,
                    y_scale,
                    this
                ](std::size_t y) mutable {
                    render_row &row = rows[y];

                    // Disallow concurrent destination data access
                    std::lock_guard destination_guard {row.destination_lock};

                    // Acquire target dynamic frame state revision
                    std::shared_ptr frame_dynamic_state = dynamic_state;

                    const camera camera = frame_dynamic_state->render_camera;

                    const bool is_new_revision = row.frame_revision != frame_dynamic_state->revision;

                    vec3 *destination = row.destination.get();
                    const vec3 *source = is_new_revision ? destination : row.source.get();

                    if (is_new_revision)
                        std::memset(destination, 0, sizeof(vec3) * width);

                    constexpr double bias_norm = (double)std::numeric_limits<std::uint64_t>::max();
                    float bias_y = (double)thread_random.next() / bias_norm;

                    float y_float = y_scale - (bias_y + y) * y_mul;
                    const vec3 base_direction = camera.forward + camera.up * vec3(y_float);
                    ray r {
                        .origin = camera.location,
                        .direction = vec3(0.0),
                    };
                    intersection intr;
                    const vec3 light_dir = vec3(0.30, 0.47, 0.80).normalized();

                    for (std::size_t x = 0; x < width; x++) {
                        float x_float = ((double)thread_random.next() / bias_norm + x) * x_mul - x_scale;
                        r.direction = (base_direction + camera.right * vec3(x_float)).normalized();

                        vec3 color;
                        if (object->intersect(r, intr)) {
                            float nv = std::clamp(light_dir.dot(intr.normal), 0.1f, 1.0f);

                            color = intr.hit_material->color * vec3(nv);
                        } else {
                            color = sky_trace_function(r.direction);
                        }

                        *destination++ = *source++ + color;
                    }

                    // Update row information, 'present' rendered data
                    {
                        std::lock_guard source_guard {row.source_lock};

                        if (is_new_revision) {
                            row.collected_count = 0;
                            row.frame_revision = frame_dynamic_state->revision;
                        }
                        row.collected_count++;
                        std::swap(row.source, row.destination);
                    }

                });
            }

            // Restart executor
            render_executor.emplace(render_height, thread_fns);
        }

        /// Stop rendering process, kill executor
        void stop_rendering() {
            render_executor.reset();
        }

        /// Things that may be changed 'on-fly'
        struct dynamic_frame_state {
            /// Frame camera
            camera render_camera {};

            /// Frame revision
            std::uint32_t revision = 0;
        };

        /// Current dynamic render state
        std::shared_ptr<dynamic_frame_state> dynamic_state = std::make_shared<dynamic_frame_state>();

        /// Rendered frame width
        std::size_t render_width;

        /// Rendered frame height
        std::size_t render_height;

        /// Rendered shape (yes, single object)
        std::unique_ptr<shape::shape> object;

        /// Sky tracing function
        std::function<vec3(vec3)> sky_trace_function;

        /// Rendered 'lines'
        std::vector<render_row> rows;

        /// Rendering task executor
        std::optional<executor> render_executor = std::nullopt;
    };
}

#endif // !defined(RT_ENGINE_HPP_)

// rt_engine.hpp
