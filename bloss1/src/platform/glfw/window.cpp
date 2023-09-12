#include "platform/glfw/window.hpp"
#include "platform/glfw/extensions.hpp"
#include "core/event.hpp"

namespace bls
{
    GlfwWindow::GlfwWindow(const str& title, const u32& width, const u32& height)
    {
        window_data.title = title;
        window_data.width = width;
        window_data.height = height;

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
        if (GLEW_ARB_debug_output)
        {
            glDebugMessageCallbackARB(&debug_callback, NULL);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        }

        else
            std::cout << "ARB Debug extension not supported\n";

        // Disable sticky keys
        glfwSetInputMode(native_window, GLFW_STICKY_KEYS, GLFW_FALSE);

        // Hide the mouse and enable unlimited movement
        // glfwSetInputMode(native_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // Disable VSync
        glfwSwapInterval(0);

        // Set callbacks
        glfwSetWindowUserPointer(native_window, &window_data);

        // Window close callback
        glfwSetWindowCloseCallback(native_window, [](GLFWwindow * window)
        {
            auto& window_data = *(WindowData*) glfwGetWindowUserPointer(window);
            WindowCloseEvent event = { };
            window_data.event_callback(event);
        });

        // Resize callback
        glfwSetFramebufferSizeCallback(native_window, [](GLFWwindow * window, i32 width, i32 height)
        {
            auto& window_data = *(WindowData*) glfwGetWindowUserPointer(window);
            window_data.width = width;
            window_data.height = height;

            WindowResizeEvent event = { (u32) width, (u32) height };
            window_data.event_callback(event);
        });

        // Key callback
        glfwSetKeyCallback(native_window, [](GLFWwindow * window, i32 key, i32, i32 action, i32)
        {
            // Key press
            if (action == GLFW_PRESS || action == GLFW_REPEAT)
            {
                auto& window_data = *(WindowData*) glfwGetWindowUserPointer(window);
                KeyPressEvent event = { (u32) key };
                window_data.event_callback(event);
            }
        });

        // Mouse callback
        glfwSetCursorPosCallback(native_window, [](GLFWwindow * window, f64 x_position, f64 y_position)
        {
            auto& window_data = *(WindowData*) glfwGetWindowUserPointer(window);
            MouseMoveEvent event = { x_position, y_position };
            window_data.event_callback(event);
        });

        // Scroll callback
        glfwSetScrollCallback(native_window, [](GLFWwindow * window, f64 x_offset, f64 y_offset)
        {
            auto& window_data = *(WindowData*) glfwGetWindowUserPointer(window);
            MouseScrollEvent event = { x_offset, y_offset };
            window_data.event_callback(event);
        });
    }

    GlfwWindow::~GlfwWindow()
    {
        glfwTerminate();
        std::cout << "glfw window destroyed successfully\n";
    }

    void GlfwWindow::update()
    {
        // Poll events
        glfwPollEvents();

        // Swap buffers
        glfwSwapBuffers(native_window);
    }

    void GlfwWindow::sleep(f64 seconds)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(static_cast<i64>(seconds * 1'000'000)));
    }

    void GlfwWindow::set_event_callback(const EventCallback& callback)
    {
        window_data.event_callback = callback;
    }

    u32 GlfwWindow::get_width() const
    {
        return window_data.width;
    }

    u32 GlfwWindow::get_height() const
    {
        return window_data.height;
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
