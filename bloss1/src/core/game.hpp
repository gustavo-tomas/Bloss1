#pragma once

/**
 * @brief The game :).
 */

#include "core/core.hpp"
#include "core/window.hpp"
#include "core/event.hpp"
#include "stages/stage.hpp"
#include "renderer/renderer.hpp"
#include "audio_engine.hpp"
#include "math/random.hpp"

namespace bls
{
    class Game
    {
        public:
            Game(const str& title, const u32& width, const u32& height);
            ~Game();

            // The game loop
            void run();

            void change_stage(Stage* new_stage);
            void on_event(Event& event);

            static Game& get(); // im sorry i can change
            Window& get_window();
            Renderer& get_renderer();
            AudioEngine& get_audio_engine();
            Random& get_random_engine();

            void set_target_fps(u32 fps);

        private:
            void on_window_close(const WindowCloseEvent& event);
            void on_window_resize(const WindowResizeEvent& event);
            void on_key_press(const KeyPressEvent& event);
            void on_mouse_scroll(const MouseScrollEvent& event);

            static Game* instance;

            std::unique_ptr<Window> window;
            std::unique_ptr<Renderer> renderer;
            std::unique_ptr<AudioEngine> audio_engine;
            std::unique_ptr<Random> random_engine;
            std::unique_ptr<Stage> stage;

            f64 target_spf;
            bool window_open, minimized;
    };
};
