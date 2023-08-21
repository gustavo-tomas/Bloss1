#pragma once

/**
 * @brief
 *
 */

#include "ecs/ecs.hpp"

namespace bls
{
    struct Collision
    {
        vec3 point_a;
        vec3 point_b;
        bool has_collision;
    };

    void resolve_collisions(ECS& ecs);
    Collision test_collision(ECS& ecs, u32 id_a, u32 id_b);
    void solve_collision(ECS& ecs, u32 id_a, u32 id_b, Collision collision);

    f32 apply_deceleration(f32 velocity, f32 deceleration, f32 mass, f32 dt);
};
