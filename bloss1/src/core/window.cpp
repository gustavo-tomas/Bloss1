#include "core/core.hpp"
#include "core/window.hpp"
#include "core/input.hpp"
#include "platform/glfw/window.hpp"
#include "platform/glfw/input.hpp"

namespace bls
{
    Window* Window::create(const str& title, const u32& width, const u32& height)
    {
        #ifdef _GLFW
        return new GlfwWindow(title, width, height);
        #else
        std::cerr << "no valid window defined\n";
        exit(1);
        #endif
    }

#ifdef _GLFW
    Input* Input::instance = new GlfwInput();
#else
    Input* Input::instance = nullptr;
#endif
};
