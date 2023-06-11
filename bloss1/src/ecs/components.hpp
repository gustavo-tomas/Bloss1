#pragma once

/**
 * @brief The components of the ECS. Might wanna split in other files if it gets too big.
 */

#include "ecs/ecs.hpp"

namespace bls
{
    struct transform : public component
    {
        transform(const i32 position = 0, const i32 rotation = 0, const i32 scale = 0)
        {
            this->position = position;
            this->rotation = rotation;
            this->scale = scale;
        }

        i32 position;
        i32 rotation;
        i32 scale;
    };

    struct model : public component
    {
        model(const i32 mesh = 0)
        {
            this->mesh = mesh;
        }

        i32 mesh;
    };
};
