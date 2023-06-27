#pragma once

/**
 * @brief The input manager interface. Each platform must implement the methods accordingly
 * and initialize the Input instance.
 */

#include "core/core.hpp"
#include "core/key_codes.hpp"

namespace bls
{
    class Input
    {
        public:
            static bool is_key_pressed(i32 keycode)
            {
                return instance->is_key_pressed_native(keycode);
            }

            static bool is_mouse_button_pressed(i32 button)
            {
                return instance->is_mouse_button_pressed_native(button);
            }

            static std::pair<f32, f32> get_mouse_position()
            {
                return instance->get_mouse_position_native();
            }

            static f32 get_mouse_x()
            {
                return instance->get_mouse_x_native();
            }

            static f32 get_mouse_y()
            {
                return instance->get_mouse_y_native();
            }

        protected:
            virtual bool is_key_pressed_native(i32 keycode) = 0;
            virtual bool is_mouse_button_pressed_native(i32 button) = 0;
            virtual std::pair<f32, f32> get_mouse_position_native() = 0;
            virtual f32 get_mouse_x_native() = 0;
            virtual f32 get_mouse_y_native() = 0;

        private:
            static Input* instance;
    };
};
