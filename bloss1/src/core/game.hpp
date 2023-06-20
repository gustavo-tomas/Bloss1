#pragma once

/**
 * @brief The game :).
 */

#include "core/core.hpp"
#include "core/window.hpp"
#include "core/event.hpp"
#include "renderer/renderer.hpp"
#include "ecs/ecs.hpp"

namespace bls
{
    class Game
    {
        public:
            Game(const str& title, const u32& width, const u32& height);
            ~Game();

            // The game loop
            void run();

            void on_event(Event& event);

            static Game& get(); // im sorry i can change
            Window& get_window();
            Renderer& get_renderer();

        private:
            void on_window_close(const WindowCloseEvent& event);
            void on_window_resize(const WindowResizeEvent& event);
            void on_key_press(const KeyPressEvent& event);
            void on_mouse_scroll(const MouseScrollEvent& event);

            static Game* instance;

            std::unique_ptr<Window> window;
            std::unique_ptr<Renderer> renderer;
            ECS* ecs;

            bool running;
            bool minimized;
    };
};
