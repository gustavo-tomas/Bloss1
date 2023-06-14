#pragma once

/**
 * @brief The entities of the ECS. Might wanna split in other files if it gets too big.
 */

#include "core/game.hpp"
#include "ecs/ecs.hpp"

namespace bls
{
    u32 player(ECS& ecs, const Transform& transform)
    {
        u32 id = ecs.get_id();

        ecs.models[id] = std::make_unique<Model>(90);
        ecs.transforms[id] = std::make_unique<Transform>(transform);

        return id;
    }
};
