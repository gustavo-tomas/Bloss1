#include "platform/glfw/input.hpp"
#include "core/game.hpp"

#include <GLFW/glfw3.h>

namespace bls
{
    Input* Input::instance = new GlfwInput();

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
