#pragma once

/**
 * @brief The game :).
 */

#include "core/core.hpp"
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
            static Game* instance;

            ECS* ecs;

            str title;
            u32 width;
            u32 height;

            f32 dt = 0.1f;
    };
};
