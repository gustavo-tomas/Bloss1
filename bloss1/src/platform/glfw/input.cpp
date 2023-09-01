#include "platform/glfw/input.hpp"
#include "core/game.hpp"

#include "glfw/include/GLFW/glfw3.h"

namespace bls
{
    bool GlfwInput::is_key_pressed_native(i32 keycode)
    {
        auto native_window = static_cast<GLFWwindow*>(Game::get().get_window().get_native_window());
        auto state = glfwGetKey(native_window, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool GlfwInput::is_mouse_button_pressed_native(i32 button)
    {
        auto native_window = static_cast<GLFWwindow*>(Game::get().get_window().get_native_window());
        auto state = glfwGetMouseButton(native_window, button);
        return state == GLFW_PRESS;
    }

    bool GlfwInput::is_joystick_button_pressed_native(i32 joystick, i32 button)
    {
        bool state = false;
        if (glfwJoystickPresent(joystick))
        {
            i32 button_count;
            auto buttons = glfwGetJoystickButtons(joystick, &button_count);
            state = buttons[button];
        }
        return state == GLFW_PRESS;
    }

    f32 GlfwInput::get_joystick_axis_value_native(i32 joystick, i32 axis)
    {
        f32 value = 0.0f;
        if (glfwJoystickPresent(joystick))
        {
            i32 axes_count;
            auto axes = glfwGetJoystickAxes(joystick, &axes_count);
            value = axes[axis];
        }
        return value;
    }

    std::pair<f32, f32> GlfwInput::get_mouse_position_native()
    {
        auto window = static_cast<GLFWwindow*>(Game::get().get_window().get_native_window());
        f64 x_pos, y_pos;
        glfwGetCursorPos(window, &x_pos, &y_pos);

        return { (f32) x_pos, (f32) y_pos };
    }

    f32 GlfwInput::get_mouse_x_native()
    {
        auto [x, y] = get_mouse_position_native();
        return x;
    }

    f32 GlfwInput::get_mouse_y_native()
    {
        auto [x, y] = get_mouse_position_native();
        return y;
    }
};
