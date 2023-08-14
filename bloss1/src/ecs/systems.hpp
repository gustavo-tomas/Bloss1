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
            // object->force += vec3(0.0f, object->mass * -9.8f, 0.0f); // @TODO: set a constant for gravity
            object->velocity += object->force / object->mass * dt;
            transforms[id]->position += object->velocity * dt;

            // Reset forces
            object->force = vec3(0.0f);
        }
    }

    i32 test_collision(Collider* col_a, Transform* trans_a, Collider* col_b, Transform* trans_b);
    i32 test_collision(BoxCollider* col_a, Transform* trans_a, SphereCollider* col_b, Transform* trans_b);
    i32 test_collision(SphereCollider* col_a, Transform* trans_a, SphereCollider* col_b, Transform* trans_b);
    i32 test_collision(BoxCollider* col_a, Transform* trans_a, BoxCollider* col_b, Transform* trans_b);

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
        // Box v. Sphere
        if (col_a->type == Collider::Box && col_b->type == Collider::Sphere)
            return test_collision(static_cast<BoxCollider*>(col_a), trans_a, static_cast<SphereCollider*>(col_b), trans_b);

        // Sphere v. Box
        else if (col_a->type == Collider::Sphere && col_b->type == Collider::Box)
            return test_collision(static_cast<BoxCollider*>(col_b), trans_b, static_cast<SphereCollider*>(col_a), trans_a);

        // Sphere v. Sphere
        else if (col_a->type == Collider::Sphere && col_b->type == Collider::Sphere)
            return test_collision(static_cast<SphereCollider*>(col_a), trans_a, static_cast<SphereCollider*>(col_b), trans_b);

        // Box v. Box
        else if (col_a->type == Collider::Box && col_b->type == Collider::Box)
            return test_collision(static_cast<BoxCollider*>(col_a), trans_a, static_cast<BoxCollider*>(col_b), trans_b);

        // Invalid colliders
        std::cerr << "invalid colliders\n";
        exit(1);
    }

    // Sphere v. Sphere
    i32 test_collision(SphereCollider* col_a, Transform* trans_a, SphereCollider* col_b, Transform* trans_b)
    {
        // Only the x is taken in account when calculating the scaled sphere radius
        auto dist = distance(trans_a->position, trans_b->position);
        if (dist < col_a->radius + col_b->radius)
            return 1;

        return 0;
    }

    // Box v. Sphere
    i32 test_collision(BoxCollider* col_a, Transform* trans_a, SphereCollider* col_b, Transform* trans_b)
    {
        // Point where the box 'begins'
        f32 min_x = trans_a->position.x - col_a->width;
        f32 min_y = trans_a->position.y - col_a->height;
        f32 min_z = trans_a->position.z - col_a->depth;

        // Point where the box 'ends'
        f32 max_x = trans_a->position.x + col_a->width;
        f32 max_y = trans_a->position.y + col_a->height;
        f32 max_z = trans_a->position.z + col_a->depth;

        f32 x = max(min_x, min(trans_b->position.x, max_x));
        f32 y = max(min_y, min(trans_b->position.y, max_y));
        f32 z = max(min_z, min(trans_b->position.z, max_z));

        // Distance squared
        f32 distance_2 = (
                             (x - trans_b->position.x) * (x - trans_b->position.x) +
                             (y - trans_b->position.y) * (y - trans_b->position.y) +
                             (z - trans_b->position.z) * (z - trans_b->position.z)
                         );

        return distance_2 < col_b->radius * col_b->radius;
    }

    // Box v. Box
    i32 test_collision(BoxCollider* col_a, Transform* trans_a, BoxCollider* col_b, Transform* trans_b)
    {
        // Point where the box 'begins'
        f32 min_x_a = trans_a->position.x - col_a->width;
        f32 min_y_a = trans_a->position.y - col_a->height;
        f32 min_z_a = trans_a->position.z - col_a->depth;

        f32 min_x_b = trans_b->position.x - col_b->width;
        f32 min_y_b = trans_b->position.y - col_b->height;
        f32 min_z_b = trans_b->position.z - col_b->depth;

        // Point where the box 'ends'
        f32 max_x_a = trans_a->position.x + col_a->width;
        f32 max_y_a = trans_a->position.y + col_a->height;
        f32 max_z_a = trans_a->position.z + col_a->depth;

        f32 max_x_b = trans_b->position.x + col_b->width;
        f32 max_y_b = trans_b->position.y + col_b->height;
        f32 max_z_b = trans_b->position.z + col_b->depth;

        return (
                   min_x_a <= max_x_b &&
                   max_x_a >= min_x_b &&
                   min_y_a <= max_y_b &&
                   max_y_a >= min_y_b &&
                   min_z_a <= max_z_b &&
                   max_z_a >= min_z_b
               );
    }
};
