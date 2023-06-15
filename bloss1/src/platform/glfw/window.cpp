#include "platform/glfw/window.hpp"

namespace bls
{
    Window* Window::create(const str& title, const u32& width, const u32& height)
    {
        return new GlfwWindow(title, width, height);
    }

    GlfwWindow::GlfwWindow(const str& title, const u32& width, const u32& height)
    {
        // Initialize GLFW
        glewExperimental = true;
        if (!glfwInit())
        {
            std::cerr << "failed to initialize GLFW\n";
            exit(1);
        }

        // GLFW window hints
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // OpenGL Version 4.6
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1); // Debug
        // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE); // Apple killed OpenGL support

        // GLFW window and context
        native_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
        if (!native_window)
        {
            std::cerr << "failed to create GLFW native_window\n";
            glfwTerminate();
            exit(1);
        }

        // Initialize GLEW
        glfwMakeContextCurrent(native_window);
        glewExperimental = true;
        if (glewInit() != GLEW_OK)
        {
            std::cerr << "failed to initialize GLEW\n";
            exit(1);
        }

        // Get native_window dimensions
        glfwGetWindowSize(native_window, (i32*) &width, (i32*) &height);

        // Set debug callbacks
        // if (GLEW_ARB_debug_output)
        // {
        //     glDebugMessageCallbackARB(&DebugOutputCallback, NULL);
        //     glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        // }

        // else
        // {
        //     std::cout << "ARB Debug extension not supported\n";
        // }

        // Ensure we can capture the escape key being pressed below
        glfwSetInputMode(native_window, GLFW_STICKY_KEYS, GL_TRUE);

        // Hide the mouse and enable unlimited movement
        // glfwSetInputMode(native_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // Disable VSync
        glfwSwapInterval(0);

        // Set callbacks
        glfwSetWindowUserPointer(native_window, this);

        // @TODO: callbacks
    }

    void GlfwWindow::update()
    {
        // Poll events
        glfwPollEvents();

        // Swap buffers
        glfwSwapBuffers(native_window);
    }

    u32 GlfwWindow::get_width() const
    {
        return width;
    }

    u32 GlfwWindow::get_height() const
    {
        return height;
    }

    f64 GlfwWindow::get_time() const
    {
        return glfwGetTime();
    }

    void* GlfwWindow::get_native_window() const
    {
        return native_window;
    }
};
