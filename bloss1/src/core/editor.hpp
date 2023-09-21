#pragma once

#include "core/window.hpp"
#include "ecs/ecs.hpp"

namespace bls
{
    class Editor
    {
        public:
            Editor(Window& window);
            ~Editor();

            void update(ECS& ecs, f32 dt);

        private:
            void render_entities(ECS& ecs);
            void push_style_vars();
            void pop_style_vars();

            Window& window;
            char file[65];
    };
};
