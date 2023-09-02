#pragma once

/**
 * @brief The entities of the ECS.
 */

#include "ecs/ecs.hpp"

namespace bls
{
    u32 player(ECS& ecs, const Transform& transform);
    u32 ball(ECS& ecs, const Transform& transform);
    u32 vampire(ECS& ecs, const Transform& transform);
    u32 abomination(ECS& ecs, const Transform& transform);
    u32 floor(ECS& ecs, const Transform& transform);
    u32 directional_light(ECS& ecs, const Transform& transform, const DirectionalLight& light);
    u32 point_light(ECS& ecs, const Transform& transform, const PointLight& light);
};
