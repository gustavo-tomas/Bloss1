#pragma once

/**
 * @brief The game :).
 */

#include "core/core.hpp"
#include "ecs/ecs.hpp"

namespace bls
{
    struct game
    {
        game(const str& title, const u32& width, const u32& height)
        {
            this->title = title;
            this->width = width;
            this->height = height;
        }

        ~game()
        {
            delete wrld;
            std::cout << "game destroyed successfully\n";
        }

        world* wrld;
        str title;
        u32 width;
        u32 height;
    };

    game* create_game(const str& title, const u32& width = 1024, const u32& height = 768);
    void destroy_game(const game& gm);

    void update_game(const game& gm);
};
