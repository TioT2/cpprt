//! Input controller implementation file

#ifndef RT_INPUT_HPP_
#define RT_INPUT_HPP_

#include "rt_common.hpp"

namespace rt {
    class key_state {
    public:
        key_state(bool is_pressed = false, bool is_changed = false) {
            base = 0
                | (is_pressed ? PRESSED_BIT : 0)
                | (is_changed ? CHANGED_BIT : 0);
        }

        bool is_pressed() const {
            return (base & PRESSED_BIT) != 0;
        }

        bool is_changed() const {
            return (base & CHANGED_BIT) != 0;
        }

        bool is_clicked() const {
            return is_pressed() && is_changed();
        }

        bool is_released() const {
            return !is_pressed() && is_changed();
        }

    private:
        constexpr static std::uint8_t PRESSED_BIT = 1;
        constexpr static std::uint8_t CHANGED_BIT = 2;

        std::uint8_t base = 0;
    };


    /// Simple input controller
    class input {
    public:
        /// Consrtuct input
        input(std::size_t key_count): keys(key_count) {

        }

        /// Update input with key state update
        void on_key(std::uint32_t key, bool is_pressed) {
            keys[key] = key_state(
                is_pressed,
                keys[key].is_changed() || (keys[key].is_pressed() != is_pressed)
            );
        }

        /// Unset 'changed' flags from all keys
        void clear_change_flags() {
            for (auto &state : keys)
                state = key_state(state.is_pressed(), false);
        }

        /// 
        key_state get_key_state(std::uint32_t key) const {
            return keys[key];
        }

        bool is_key_pressed(std::uint32_t key) const {
            return get_key_state(key).is_pressed();
        }

        bool is_key_changed(std::uint32_t key) const {
            return get_key_state(key).is_changed();
        }

        bool is_key_clicked(std::uint32_t key) const {
            return get_key_state(key).is_clicked();
        }

        bool is_key_released(std::uint32_t key) const {
            return get_key_state(key).is_released();
        }

    private:
        std::vector<key_state> keys;
    };
}

#endif // !defined(RT_INPUT_HPP_)

// rt_input.hpp
