#pragma once

/**
 * @brief The glfw implementation of the Input class.
 */

#include "core/input.hpp"

namespace bls
{
    class GlfwInput : public Input
    {
        protected:
            bool is_key_pressed_native(i32 keycode) override;
            bool is_mouse_button_pressed_native(i32 button) override;
            bool is_joystick_button_pressed_native(i32 joystick, i32 button) override;
            f32 get_joystick_axis_value_native(i32 joystick, i32 axis) override;
            std::pair<f32, f32> get_mouse_position_native() override;
            f32 get_mouse_x_native() override;
            f32 get_mouse_y_native() override;
    };
};  // namespace bls
