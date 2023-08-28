#include "ecs/systems.hpp"
#include "ecs/systems/physics_system.hpp"

#define GRAVITY 9.8f

// @TODO: finish and cleanup
// @TODO: use continuous collision detection
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
                object->force += vec3(0.0f, object->mass * -GRAVITY, 0.0f);
                object->velocity += (object->force / object->mass) * dt;

                // Apply deceleration
                object->velocity.x = apply_deceleration(object->velocity.x, 20.0f, object->mass, dt);
                object->velocity.y = apply_deceleration(object->velocity.y, 1.0f, object->mass, dt);
                object->velocity.z = apply_deceleration(object->velocity.z, 20.0f, object->mass, dt);

                transforms[id]->position += object->velocity * dt;
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

            // 1) find point 'pbox' on box the closest to the sphere centre.
            vec3 closest_point_aabb;

            // For each coordinate axis, if the point coordinate value is
            // outside box, clamp it to the box, else keep it as is
            for (u32 i = 0; i < 3; i++)
            {
                f32 v = trans_b->position[i];
                v = max(v, min_aabb[i]);
                v = min(v, max_aabb[i]);
                closest_point_aabb[i] = v;
            }

            // 2) if 'pbox' is outside the sphere no collision.
            f32 dist_aabb_to_sphere = distance(closest_point_aabb, trans_b->position);
            if (dist_aabb_to_sphere < col_b->radius)
            {
                // 3) find point 'pshpere' on sphere surface the closest to point 'pbox'.
                vec3 closest_point_sphere = trans_b->position + normalize(closest_point_aabb - trans_b->position) * col_b->radius;

                collision.point_a = closest_point_sphere;
                collision.point_b = closest_point_aabb;
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

        auto object_a = ecs.physics_objects[id_a].get();
        auto object_b = ecs.physics_objects[id_b].get();

        vec3 delta = collision.point_a - collision.point_b;
        f32 dist = length(delta);
        vec3 normal = delta / dist;

        if (!collider_a->immovable)
        {
            trans_a->position += normal * dist * 0.5f;
            object_a->velocity += normal * dot(object_a->velocity, delta);
        }

        if (!collider_b->immovable)
        {
            trans_b->position -= normal * dist * 0.5f;
            object_b->velocity -= normal * dot(object_b->velocity, delta);
        }
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
