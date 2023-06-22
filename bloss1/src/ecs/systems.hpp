#pragma once

/**
 * @brief The systems of the ECS. Might wanna split in other files if it gets too big.
 */

#include "ecs/ecs.hpp"

namespace bls
{
    void render_system(ECS& ecs, f32 dt)
    {
        auto& models = ecs.models;
        for (auto& [id, model] : models)
        {
            model->mesh += 200 * dt;
        }
    }

    void transform_system(ECS& ecs, f32 dt)
    {
        auto& transforms = ecs.transforms;
        for (auto& [id, transform] : transforms)
        {
            transform->scale += 400 * dt;
        }
    }
};
