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
            // Do not apply forces to immovable ojbects
            if (!colliders[id]->immovable)
            {
                // Apply forces
                object->force += vec3(0.0f, object->mass * -9.8f, 0.0f); // @TODO: set a constant for gravity
                object->velocity += (object->force / object->mass) * dt;

                // Apply deceleration
                object->velocity.x = apply_deceleration(object->velocity.x, 20.0f, object->mass, dt);
                object->velocity.y = apply_deceleration(object->velocity.y, 1.0f, object->mass, dt);
                object->velocity.z = apply_deceleration(object->velocity.z, 20.0f, object->mass, dt);

                transforms[id]->position += object->velocity * dt;
            }

            // @TODO: use continuous collision detection
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
            vec3 min_aabb = vec3(0.0f);
            min_aabb.x = trans_a->position.x - col_a->width;
            min_aabb.y = trans_a->position.y - col_a->height;
            min_aabb.z = trans_a->position.z - col_a->depth;

            // Point where the box 'ends'
            vec3 max_aabb = vec3(0.0f);
            max_aabb.x = trans_a->position.x + col_a->width;
            max_aabb.y = trans_a->position.y + col_a->height;
            max_aabb.z = trans_a->position.z + col_a->depth;

            // AABB closest point to the sphere
            vec3 closest_point_aabb = vec3(0.0f);
            closest_point_aabb.x = clamp(trans_b->position.x, min_aabb.x, max_aabb.x);
            closest_point_aabb.y = clamp(trans_b->position.y, min_aabb.y, max_aabb.y);
            closest_point_aabb.z = clamp(trans_b->position.z, min_aabb.z, max_aabb.z);

            // Sphere closest point to AABB
            vec3 vector_to_closest = normalize(closest_point_aabb - trans_b->position);
            vec3 closest_point_sphere = trans_b->position + vector_to_closest * col_b->radius;

            f32 dist_aabb_to_sphere = distance(trans_b->position, closest_point_aabb);
            if (dist_aabb_to_sphere < col_b->radius)
            {
                collision.point_a = closest_point_aabb;
                collision.point_b = closest_point_sphere;
                collision.has_collision = true;
            }

            return collision;
        }

        // Sphere v. Sphere
        else if (collider_a->type == Collider::Sphere && collider_b->type == Collider::Sphere)
        {
            auto col_a = static_cast<SphereCollider*>(collider_a);
            auto col_b = static_cast<SphereCollider*>(collider_b);

            // Insert tolerance to avoid equal points
            const f32 TOL = 0.002;
            f32 dist = distance(trans_a->position, trans_b->position);
            if (dist < col_a->radius + col_b->radius - TOL)
            {
                // SphereA closest point to SphereB
                vec3 vector_to_center_b = normalize(trans_b->position - trans_a->position);
                vec3 vector_to_center_a = normalize(trans_a->position - trans_b->position);
                vec3 closest_point_sphere_a = trans_b->position + vector_to_center_a * col_b->radius;
                vec3 closest_point_sphere_b = trans_a->position + vector_to_center_b * col_a->radius;

                collision.point_a = closest_point_sphere_a;
                collision.point_b = closest_point_sphere_b;
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
            vec3 min_aabb_a = vec3(0.0f);
            min_aabb_a.x = trans_a->position.x - col_a->width;
            min_aabb_a.y = trans_a->position.y - col_a->height;
            min_aabb_a.z = trans_a->position.z - col_a->depth;

            vec3 min_aabb_b = vec3(0.0f);
            min_aabb_b.x = trans_b->position.x - col_b->width;
            min_aabb_b.y = trans_b->position.y - col_b->height;
            min_aabb_b.z = trans_b->position.z - col_b->depth;

            // Point where the box 'ends'
            vec3 max_aabb_a = vec3(0.0f);
            max_aabb_a.x = trans_a->position.x + col_a->width;
            max_aabb_a.y = trans_a->position.y + col_a->height;
            max_aabb_a.z = trans_a->position.z + col_a->depth;

            vec3 max_aabb_b = vec3(0.0f);
            max_aabb_b.x = trans_b->position.x + col_b->width;
            max_aabb_b.y = trans_b->position.y + col_b->height;
            max_aabb_b.z = trans_b->position.z + col_b->depth;

            bool intersecting = (min_aabb_a.x <= max_aabb_b.x && max_aabb_a.x >= min_aabb_b.x &&
                                 min_aabb_a.y <= max_aabb_b.y && max_aabb_a.y >= min_aabb_b.y &&
                                 min_aabb_a.z <= max_aabb_b.z && max_aabb_a.z >= min_aabb_b.z);

            // @TODO: finish collision response
            if (intersecting)
            {
                // collision.point_a = ;
                // collision.point_b = ;
                // collision.has_collision = true;
            }

            return collision;
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

        vec3 normal = collision.point_a - collision.point_b;
        f32 dist = length(normal);

        normal /= dist;

        if (!collider_a->immovable)
            trans_a->position += normal * dist * 0.5f;

        if (!collider_b->immovable)
            trans_b->position -= normal * dist * 0.5f;
    }

    f32 apply_deceleration(f32 velocity, f32 deceleration, f32 mass, f32 dt)
    {
        if (velocity > 0)
        {
            velocity -= (deceleration / mass) * dt;
            return max(velocity, 0.0f);
        }

        else if (velocity < 0)
        {
            velocity += (deceleration / mass) * dt;
            return min(velocity, 0.0f);
        }

        return 0.0f;
    }
};
