#pragma once

/**
 * @brief The game :).
 */

#include "audio_engine/audio_engine.hpp"
#include "core/core.hpp"
#include "core/editor.hpp"
#include "core/event.hpp"
#include "core/window.hpp"
#include "math/random.hpp"
#include "renderer/renderer.hpp"
#include "stages/stage.hpp"

namespace bls
{
    class Game
    {
        public:
            Game(const str &title, const u32 &width, const u32 &height);
            ~Game();

            // The game loop
            void run();

            void change_stage(Stage *new_stage);
            void on_event(Event &event);

            static Game &get();  // im sorry i can change
            Window &get_window();
            Renderer &get_renderer();
            Editor &get_editor();
            AudioEngine &get_audio_engine();
            Random &get_random_engine();
            Stage &get_curr_stage();

            void set_target_fps(u32 fps);

        private:
            void on_window_close(const WindowCloseEvent &event);
            void on_window_resize(const WindowResizeEvent &event);
            void on_key_press(const KeyPressEvent &event);
            void on_mouse_scroll(const MouseScrollEvent &event);

            static Game *instance;

            std::unique_ptr<Window> window;
            std::unique_ptr<Renderer> renderer;
            std::unique_ptr<Editor> editor;
            std::unique_ptr<AudioEngine> audio_engine;
            std::unique_ptr<Random> random_engine;
            std::unique_ptr<Stage> stage;

            f64 target_spf;
            bool window_open, minimized;
    };
};  // namespace bls
