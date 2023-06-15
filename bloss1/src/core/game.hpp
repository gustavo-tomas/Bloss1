#pragma once

/**
 * @brief The game :).
 */

#include "core/core.hpp"
#include "core/window.hpp"
#include "core/event.hpp"
#include "ecs/ecs.hpp"

namespace bls
{
    class Game
    {
        public:
            Game(const str& title, const u32& width, const u32& height);
            ~Game();

            static Game& get(); // im sorry i can change

            // The game loop
            void run();

        private:
            void on_window_close(const WindowCloseEvent& event);
            void on_window_resize(const WindowResizeEvent& event);
            void on_key_press(const KeyPressEvent& event);
            void on_mouse_scroll(const MouseScrollEvent& event);

            static Game* instance;

            Window* window;
            ECS* ecs;

            str title;
            u32 width;
            u32 height;

            bool running;
    };
};
