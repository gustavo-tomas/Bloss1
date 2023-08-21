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

    void resolve_collisions(ECS& ecs, f32 dt);

    Collision test_collision(Collider* col_a, Transform* trans_a, Collider* col_b, Transform* trans_b);
    Collision test_collision(BoxCollider* col_a, Transform* trans_a, SphereCollider* col_b, Transform* trans_b);
    Collision test_collision(SphereCollider* col_a, Transform* trans_a, SphereCollider* col_b, Transform* trans_b);
    Collision test_collision(BoxCollider* col_a, Transform* trans_a, BoxCollider* col_b, Transform* trans_b);

    void solve_collision(Transform* trans_a, PhysicsObject* object_a, Collider* collider_a,
                         Transform* trans_b, PhysicsObject* object_b, Collider* collider_b,
                         Collision collision, f32 dt);

    void solve_collision(Transform* trans_a, PhysicsObject* object_a, SphereCollider* collider_a,
                         Transform* trans_b, PhysicsObject* object_b, SphereCollider* collider_b,
                         Collision collision, f32 dt);

    void solve_collision(Transform* trans_a, PhysicsObject* object_a, BoxCollider* collider_a,
                         Transform* trans_b, PhysicsObject* object_b, SphereCollider* collider_b,
                         Collision collision, f32 dt);

    void solve_collision(Transform* trans_a, PhysicsObject* object_a, Collider* collider_a,
                         Transform* trans_b, PhysicsObject* object_b, Collider* collider_b,
                         Collision collision, f32 dt);

    f32 apply_deceleration(f32 velocity, f32 deceleration, f32 mass, f32 dt);
};
