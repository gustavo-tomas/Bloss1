#pragma once

#include "core/window.hpp"
#include "ecs/ecs.hpp"

namespace bls
{
    class Editor
    {
        public:
            typedef std::pair<u32, str> pass_position_name_pair;
            struct Stats
            {
                u32 vertices = 0;
                f32 framerate = 0.0f;
                f32 ms_per_frame = 0.0f;
            };

            struct Configs
            {
                std::vector<pass_position_name_pair> render_passes;
            };

            Editor(Window& window);
            ~Editor();

            void update(ECS& ecs, f32 dt);
            void update_configs(Configs configs);

            Stats app_stats;
            Configs app_configs;

        private:
            void render_entities(ECS& ecs);
            void render_status();
            void render_config();

            void push_style_vars();
            void pop_style_vars();

            Window& window;
            char save_file[65];
    };
};
