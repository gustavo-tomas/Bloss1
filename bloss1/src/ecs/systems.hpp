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
            // @TODO: for now, do nothing
        }
    }

    // @TODO: finish
    void resolve_collisions(ECS& ecs, f32 dt);
    void physics_system(ECS& ecs, f32 dt)
    {
        resolve_collisions(ecs, dt);

        auto& transforms = ecs.transforms;
        auto& objects = ecs.physics_objects;
        for (auto& [id, object] : objects)
        {
            // Apply forces
            object->force += vec3(0.0f, object->mass * -9.8f, 0.0f); // @TODO: set a constant for gravity
            object->velocity += object->force / object->mass * dt;
            transforms[id]->position += object->velocity * dt;

            // Reset forces
            object->force = vec3(0.0f);
        }
    }

    i32 test_collision(Collider* col_a, Transform* trans_a, Collider* col_b, Transform* trans_b);
    i32 test_collision(PlaneCollider* col_a, Transform* trans_a, SphereCollider* col_b, Transform* trans_b);
    i32 test_collision(SphereCollider* col_a, Transform* trans_a, SphereCollider* col_b, Transform* trans_b);

    void resolve_collisions(ECS& ecs, f32 dt)
    {
        auto& colliders = ecs.colliders;
        auto& transforms = ecs.transforms;
        for (auto& [id_a, collider_a] : colliders)
        {
            for (auto& [id_b, collider_b] : colliders)
            {
                // Test only unique pairs
                if (id_a == id_b)
                    break;

                bool collision = test_collision(collider_a.get(), transforms[id_a].get(), collider_b.get(), transforms[id_b].get());
                if (collision)
                {
                    std::cout << "COLLISION: " << id_a << ", " << id_b << std::endl;
                    // solve_collision(a, b, points);
                }
            }
        }
    }

    i32 test_collision(Collider* col_a, Transform* trans_a, Collider* col_b, Transform* trans_b)
    {
        // Plane v. Sphere
        if (col_a->type == Collider::Plane && col_b->type == Collider::Sphere)
            return test_collision(static_cast<PlaneCollider*>(col_a), trans_a, static_cast<SphereCollider*>(col_b), trans_b);

        // Sphere v. Plane
        else if (col_a->type == Collider::Sphere && col_b->type == Collider::Plane)
            return test_collision(static_cast<PlaneCollider*>(col_b), trans_b, static_cast<SphereCollider*>(col_a), trans_a);

        // Sphere v. Sphere
        else if (col_a->type == Collider::Sphere && col_b->type == Collider::Sphere)
            return test_collision(static_cast<SphereCollider*>(col_a), trans_a, static_cast<SphereCollider*>(col_b), trans_b);

        // Invalid colliders
        std::cerr << "invalid colliders\n";
        exit(1);
    }

    // Plane v. Sphere
    i32 test_collision(PlaneCollider* col_a, Transform* trans_a, SphereCollider* col_b, Transform* trans_b)
    {
        return 0;
    }

    // Sphere v. Sphere
    i32 test_collision(SphereCollider* col_a, Transform* trans_a, SphereCollider* col_b, Transform* trans_b)
    {
        return 0;
    }
};
