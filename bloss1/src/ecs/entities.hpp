#pragma once

/**
 * @brief The entities of the ECS. Might wanna split in other files if it gets too big.
 */

#include "core/game.hpp"
#include "ecs/ecs.hpp"
#include "renderer/buffers.hpp"

namespace bls
{
    u32 player(ECS& ecs, const Transform& transform)
    {
        u32 id = ecs.get_id();

        auto model = Model::create("player", "bloss1/assets/models/box/planter_box_01_2k.gltf", false);

        ecs.models[id] = std::make_unique<ModelComponent>(model.get());
        ecs.transforms[id] = std::make_unique<Transform>(transform);

        return id;
    }
};
