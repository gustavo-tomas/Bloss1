#pragma once

/**
 * @brief The systems of the ECS. Might wanna split in other files if it gets too big.
 */

#include "ecs/ecs.hpp"
#include "ecs/components.hpp"

namespace bls
{
    void render_system(const world& wrld, const f32 dt)
    {
        auto entities = query_entities(wrld, { "model" });
        for (auto& entity : entities)
        {
            model* mdl = dynamic_cast<model*>(entity->components["model"]);
            mdl->mesh += 200 * dt;

            std::cout << "mesh: " << mdl->mesh << "\n";
        }
    }

    void transform_system(const world& wrld, const f32 dt)
    {
        auto entities = query_entities(wrld, { "transform" });
        for (auto& entity : entities)
        {
            transform* trns = dynamic_cast<transform*>(entity->components["transform"]);
            trns->position += 30 * dt;
            trns->scale += 50 * dt;

            std::cout << "pos: " << trns->position << " scale: " << trns->scale << "\n";
        }
    }
};
