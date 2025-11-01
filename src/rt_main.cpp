//! Main project file
//! Fun fact: this file is **the only** environment-dependend file here

#include <print>

#include <SDL3/SDL.h>

#include "rt.hpp"

// Main function
int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::println("SDL Initialization failed: {}", SDL_GetError());
        return 0;
    }

    SDL_Window *window = SDL_CreateWindow("RT", 800, 600, 0);
    if (window == nullptr) {
        std::println("SDL Window creation failed: {}", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    auto sphere1_material = std::make_shared<rt::material>(rt::vec3(0.1f, 0.1f, 0.9f));
    auto sphere2_material = std::make_shared<rt::material>(rt::vec3(0.6f, 0.6f, 0.6f));
    auto plane_material = std::make_shared<rt::material>(rt::vec3(0.80f, 0.47f, 0.30f));

    auto scene = std::make_unique<rt::shape::scene>();

    // Fill scene with objects
    *scene
        << std::make_unique<rt::shape::sphere>(rt::vec3(0.0f), 1.0f, sphere1_material)
        << std::make_unique<rt::shape::sphere>(rt::vec3(1.4f), 0.3f, sphere2_material)
        << std::make_unique<rt::shape::plane>(
            rt::vec3(0.0f, 0.0f, 0.0f),
            rt::vec3(0.0f, 1.0f, 0.0f),
            plane_material
        )
        ;

    rt::engine engine {std::move(scene)};

    rt::input input {SDL_SCANCODE_COUNT};
    rt::timer timer;

    // Current camera state
    rt::camera camera = rt::camera::from_loc_dir_up(
        rt::vec3(10.0f, 10.0f, 10.0f),
        rt::vec3(-1.0f, -1.0f, -1.0f).normalized(),
        rt::vec3(0.0f, 1.0f, 0.0f)
    );

    // Update engine camera
    engine.set_camera(camera);

    bool do_quit = false;
    while (!do_quit) {
        SDL_Event event = {};

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                do_quit = true;
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                engine.set_render_resolution(
                    event.window.data1,
                    event.window.data2
                );
                break;

            case SDL_EVENT_KEY_DOWN:
                input.on_key(event.key.scancode, true);
                break;

            case SDL_EVENT_KEY_UP:
                input.on_key(event.key.scancode, false);
                break;
            }
        }

        timer.update();

        // Control!
        {
            auto axis = [&]( SDL_Scancode pos, SDL_Scancode neg ){
                return (float)input.is_key_pressed(pos) - (float)input.is_key_pressed(neg);
            };

            bool camera_changed = false;

            rt::vec3 move_axis {
                axis(SDL_SCANCODE_W, SDL_SCANCODE_S),
                axis(SDL_SCANCODE_D, SDL_SCANCODE_A),
                axis(SDL_SCANCODE_R, SDL_SCANCODE_F)
            };

            // New camera properties
            rt::vec3 new_location = camera.location;
            rt::vec3 new_direction = camera.forward;

            if (move_axis.length2() >= 0.01f) {
                camera_changed = true;
                move_axis = move_axis * rt::vec3(timer.get_delta_time() * 10.0f);

                // Alter camera location
                new_location = camera.location
                    + camera.forward * rt::vec3(move_axis.x)
                    + camera.right   * rt::vec3(move_axis.y)
                    + camera.up      * rt::vec3(move_axis.z)
                ;
            }

            rt::vec3 rotate_axis {
                axis(SDL_SCANCODE_RIGHT, SDL_SCANCODE_LEFT),
                axis(SDL_SCANCODE_DOWN, SDL_SCANCODE_UP),
                0.0f
            };

            if (rotate_axis.length2() >= 0.01f) {
                camera_changed = true;
                rotate_axis = rotate_axis * rt::vec3(timer.get_delta_time() * 2.5f);

                rt::vec3 dir = camera.forward;

                float azimuth = std::acos(dir.y);
                float elevator = dir.z / std::abs(dir.z) * std::acos(
                    dir.x / std::sqrt(dir.x * dir.x + dir.z * dir.z)
                );

                elevator += rotate_axis.x;
                azimuth += rotate_axis.y;

                azimuth = std::clamp(azimuth, 0.01f, std::numbers::pi_v<float> - 0.01f);

                new_direction = rt::vec3(
                    std::sin(azimuth) * std::cos(elevator),
                    std::cos(azimuth),
                    std::sin(azimuth) * std::sin(elevator)
                );
            }

            if (camera_changed) {
                camera = rt::camera::from_loc_dir_up(
                    new_location,
                    new_direction,
                    rt::vec3(0.0, 1.0, 0.0)
                );
                engine.set_camera(camera);
            }
        }

        input.clear_change_flags();

        // Render
        SDL_Surface *surface = SDL_GetWindowSurface(window);

        if (!SDL_MUSTLOCK(surface) || SDL_LockSurface(surface)) {

            // Display only if pixelformats is good enough
            if (surface->format == SDL_PIXELFORMAT_BGRX32)
                engine.display_frame(
                    reinterpret_cast<std::byte *>(surface->pixels),
                    surface->pitch
                );

            if (SDL_MUSTLOCK(surface))
                SDL_UnlockSurface(surface);
        }

        SDL_UpdateWindowSurface(window);
    }

    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}

// rt_main.cpp
