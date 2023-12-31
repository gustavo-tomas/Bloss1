#pragma once

#include "config.hpp"
#include "core/window.hpp"
#include "ecs/ecs.hpp"

namespace bls
{
    class Editor
    {
        public:
            Editor(Window &window);
            ~Editor();

            void update(ECS &ecs, f32 dt);

        private:
            void render_entities(ECS &ecs);
            void render_status();
            void render_config();
            void render_console();

            void display_editable_params(PassConfig &pass);

            void push_style_vars();
            void pop_style_vars();

            Window &window;
            char save_file[65], config_file[65], skybox_file[65];
    };
};  // namespace bls
