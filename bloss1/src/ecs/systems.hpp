#pragma once

/**
 * @brief The systems of the ECS. Might wanna split in other files if it gets too big.
 */

#include "ecs/ecs.hpp"

namespace bls
{
    void render_system(ECS& ecs, const f32& dt)
    {
        std::cout << "\nRenderSystem:\n";

        auto& models = ecs.models;
        for (auto& [id, model] : models)
        {
            model->mesh += 200 * dt;

            std::cout << " - mesh: " << model->mesh << "\n";
        }
    }

    void transform_system(ECS& ecs, const f32& dt)
    {
        std::cout << "\nTransformSystem:\n";

        auto& transforms = ecs.transforms;
        for (auto& [id, transform] : transforms)
        {
            transform->scale += 400 * dt;

            std::cout << " - scale: " << transform->scale << "\n";
        }
    }
};
