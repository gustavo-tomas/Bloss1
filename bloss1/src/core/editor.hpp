#pragma once

#include "core/window.hpp"

namespace bls
{
    class Editor
    {
        public:
            Editor(Window& window);
            ~Editor();

            void update(f32 dt);

        private:
            void push_style_vars();
            void pop_style_vars();

            Window& window;
    };
};
