#include "ecs/systems.hpp"
#include "ecs/systems/physics_system.hpp"

// @TODO: finish and cleanup
namespace bls
{
    void physics_system(ECS& ecs, f32 dt)
    {
        resolve_collisions(ecs);

        auto& transforms = ecs.transforms;
        auto& objects = ecs.physics_objects;
        auto& colliders = ecs.colliders;
        for (auto& [id, object] : objects)
        {
            // Negative mass == floor
            if (object->mass > 0.0f)
            {
                // Apply forces
                object->force += vec3(0.0f, object->mass * -9.8f, 0.0f); // @TODO: set a constant for gravity
                object->velocity += object->force / object->mass * dt;

                // Apply deceleration
                object->velocity.x = apply_deceleration(object->velocity.x, 10.0f, object->mass, dt);
                object->velocity.z = apply_deceleration(object->velocity.z, 10.0f, object->mass, dt);

                transforms[id]->position += object->velocity * dt;
            }

            // Prevent objects from clipping the floor
            f32 vertical_side = 0.0f;
            if (colliders[id]->type == Collider::ColliderType::Sphere)
                vertical_side = static_cast<SphereCollider*>(colliders[id].get())->radius;

            else if (colliders[id]->type == Collider::ColliderType::Box)
                vertical_side = static_cast<BoxCollider*>(colliders[id].get())->height / 2.0f;

            const f32 TOL = 0.025f;
            if (transforms[id]->position.y - vertical_side <= 0.0f + TOL)
            {
                transforms[id]->position.y = vertical_side;
                object->velocity.y = 0.0f;
            }

            // Reset forces
            object->force = vec3(0.0f);
        }
    }

    void resolve_collisions(ECS& ecs)
    {
        auto& colliders = ecs.colliders;
        for (auto& [id_a, collider_a] : colliders)
        {
            // Assume no collision happens
            collider_a->color = vec3(0.0f);

            for (auto& [id_b, collider_b] : colliders)
            {
                // Test only unique pairs
                if (id_a == id_b)
                    break;

                // Solve collision
                auto collision = test_collision(ecs, id_a, id_b);
                if (collision.has_collision)
                {
                    collider_a->color = { 1.0f, 0.0f, 0.0f };
                    collider_b->color = { 1.0f, 0.0f, 0.0f };
                    solve_collision(ecs, id_a, id_b, collision);
                }
            }
        }
    }

    // Collision tester
    // -----------------------------------------------------------------------------------------------------------------
    Collision test_collision(ECS& ecs, u32 id_a, u32 id_b)
    {
        auto collider_a = ecs.colliders[id_a].get();
        auto collider_b = ecs.colliders[id_b].get();

        auto trans_a = ecs.transforms[id_a].get();
        auto trans_b = ecs.transforms[id_b].get();

        Collision collision = { };

        // Sphere v. Box
        if (collider_a->type == Collider::Sphere && collider_b->type == Collider::Box)
        {
            // Swap
            auto temp = collider_a;
            collider_a = collider_b;
            collider_b = temp;
        }

        // Box v. Sphere
        if (collider_a->type == Collider::Box && collider_b->type == Collider::Sphere)
        {
            auto col_a = static_cast<BoxCollider*>(collider_a);
            auto col_b = static_cast<SphereCollider*>(collider_b);

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

            // @TODO
            if (distance_2 < col_b->radius * col_b->radius)
            {
                // Calculate the penetration depth along each axis
                f32 penetrationX = 0.0, penetrationY = 0.0, penetrationZ = 0.0;

                if (trans_b->position.x < min_x)
                    penetrationX = min_x - trans_b->position.x;
                else if (trans_b->position.x > max_x)
                    penetrationX = trans_b->position.x - max_x;

                if (trans_b->position.y < min_y)
                    penetrationY = min_y - trans_b->position.y;
                else if (trans_b->position.y > max_y)
                    penetrationY = col_b->radius - (trans_b->position.y - max_y);

                if (trans_b->position.z < min_z)
                    penetrationZ = min_z - trans_b->position.z;
                else if (trans_b->position.z > max_z)
                    penetrationZ = trans_b->position.z - max_z;

                collision.point_a = vec3(penetrationX, penetrationY, penetrationZ);
                collision.point_b = vec3(0.0f);
                collision.has_collision = true;
            }

            return collision;
        }

        // Sphere v. Sphere
        else if (collider_a->type == Collider::Sphere && collider_b->type == Collider::Sphere)
        {
            auto col_a = static_cast<SphereCollider*>(collider_a);
            auto col_b = static_cast<SphereCollider*>(collider_b);

            auto dist = distance(trans_a->position, trans_b->position);
            if (dist < col_a->radius + col_b->radius)
            {
                vec3 pa = trans_a->position + (dist - col_b->radius);
                vec3 pb = trans_b->position + (dist - col_a->radius);

                collision.point_a = pa;
                collision.point_b = pb;
                collision.has_collision = true;
            }

            return collision;
        }

        // Box v. Box
        else if (collider_a->type == Collider::Box && collider_b->type == Collider::Box)
        {
            auto col_a = static_cast<BoxCollider*>(collider_a);
            auto col_b = static_cast<BoxCollider*>(collider_b);

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

            return { };

            // return (
            //            min_x_a <= max_x_b &&
            //            max_x_a >= min_x_b &&
            //            min_y_a <= max_y_b &&
            //            max_y_a >= min_y_b &&
            //            min_z_a <= max_z_b &&
            //            max_z_a >= min_z_b
            //        );
        }

        // Invalid colliders
        std::cerr << "invalid colliders\n";
        exit(1);
    }

    // Collision solver
    // -----------------------------------------------------------------------------------------------------------------
    void solve_collision(ECS& ecs, u32 id_a, u32 id_b, Collision collision)
    {
        auto collider_a = ecs.colliders[id_a].get();
        auto collider_b = ecs.colliders[id_b].get();

        auto trans_a = ecs.transforms[id_a].get();
        auto trans_b = ecs.transforms[id_b].get();

        // Sphere v. Box
        if (collider_a->type == Collider::Sphere && collider_b->type == Collider::Box)
        {
            // Swap
            auto temp = collider_a;
            collider_a = collider_b;
            collider_b = temp;
        }

        // Box v. Sphere
        if (collider_a->type == Collider::Box && collider_b->type == Collider::Sphere)
        {
            // Move the sphere to resolve overlap
            trans_b->position += vec3(0.0f, collision.point_a.y, 0.0f);
            return;
        }

        // Sphere v. Sphere
        else if (collider_a->type == Collider::Sphere && collider_b->type == Collider::Sphere)
        {
            auto col_a = static_cast<SphereCollider*>(collider_a);
            auto col_b = static_cast<SphereCollider*>(collider_b);

            vec3 normal = collision.point_a - collision.point_b;
            f32 dist = glm::length(normal);

            normal /= dist;

            // Calculate the separation distance (overlap)
            f32 overlap = col_a->radius + col_b->radius - dist;

            // Move the spheres to resolve overlap
            trans_a->position += normal * overlap * 0.5f;
            trans_b->position -= normal * overlap * 0.5f;
            return;
        }

        // Box v. Box
        // else if (collider_a->type == Collider::Box && collider_b->type == Collider::Box)
        //     return solve_collision(trans_a, object_a, static_cast<BoxCollider*>(collider_a),
        //                            trans_b, static_cast<BoxCollider*>(collider_b),
        //                            collision, dt);
    }

    f32 apply_deceleration(f32 velocity, f32 deceleration, f32 mass, f32 dt)
    {
        if (velocity > 0)
        {
            velocity -= deceleration / mass * dt;
            return max(velocity, 0.0f);
        }

        else if (velocity < 0)
        {
            velocity += deceleration / mass * dt;
            return min(velocity, 0.0f);
        }

        return 0.0f;
    }
};
