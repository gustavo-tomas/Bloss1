#pragma once

/**
 * @brief The glfw window that implements the window interface.
 */

#include "core/window.hpp"

#include <GL/glew.h> // Include glew before glfw
#include <GLFW/glfw3.h>

namespace bls
{
    class GlfwWindow : public Window
    {
        public:
            GlfwWindow(const str& title, const u32& width, const u32& height);

            void update() override;

            u32 get_width() const override;
            u32 get_height() const override;

            f64 get_time() const override;
            void* get_native_window() const override;

        private:
            GLFWwindow* native_window;

            str title;
            u32 width;
            u32 height;
    };
};
