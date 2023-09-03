#include "ecs/systems.hpp"

#define GRAVITY 9.8f

// @TODO: use continuous collision detection
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

        auto trans_a = *ecs.transforms[id_a].get();
        auto trans_b = *ecs.transforms[id_b].get();

        trans_a.position += collider_a->offset;
        trans_b.position += collider_b->offset;

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
            min_aabb.x = trans_a.position.x - col_a->width;
            min_aabb.y = trans_a.position.y - col_a->height;
            min_aabb.z = trans_a.position.z - col_a->depth;

            // Point where the box 'ends'
            vec3 max_aabb = vec3(0.0f);
            max_aabb.x = trans_a.position.x + col_a->width;
            max_aabb.y = trans_a.position.y + col_a->height;
            max_aabb.z = trans_a.position.z + col_a->depth;

            // 1) find point 'pbox' on box the closest to the sphere centre.
            vec3 closest_point_aabb;

            // For each coordinate axis, if the point coordinate value is
            // outside box, clamp it to the box, else keep it as is
            for (u32 i = 0; i < 3; i++)
                closest_point_aabb[i] = clamp(trans_b.position[i], min_aabb[i], max_aabb[i]);

            // 2) if 'pbox' is outside the sphere no collision.
            f32 dist_aabb_to_sphere = distance(closest_point_aabb, trans_b.position);
            if (dist_aabb_to_sphere < col_b->radius)
            {
                // 3) find point 'pshpere' on sphere surface the closest to point 'pbox'.
                vec3 closest_point_sphere = trans_b.position + normalize(closest_point_aabb - trans_b.position) * col_b->radius;

                if (length(closest_point_sphere - closest_point_aabb) > 0.0f)
                {
                    collision.point_a = closest_point_sphere;
                    collision.point_b = closest_point_aabb;
                    collision.has_collision = true;
                }
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
            f32 dist = distance(trans_a.position, trans_b.position);
            if (dist < col_a->radius + col_b->radius - TOL)
            {
                // SphereA closest point to SphereB
                vec3 vector_to_center_b = normalize(trans_b.position - trans_a.position);
                vec3 vector_to_center_a = normalize(trans_a.position - trans_b.position);
                vec3 closest_point_sphere_a = trans_b.position + vector_to_center_a * col_b->radius;
                vec3 closest_point_sphere_b = trans_a.position + vector_to_center_b * col_a->radius;

                if (length(closest_point_sphere_a - closest_point_sphere_b) > 0.0f)
                {
                    collision.point_a = closest_point_sphere_a;
                    collision.point_b = closest_point_sphere_b;
                    collision.has_collision = true;
                }
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
            min_aabb_a.x = trans_a.position.x - col_a->width;
            min_aabb_a.y = trans_a.position.y - col_a->height;
            min_aabb_a.z = trans_a.position.z - col_a->depth;

            vec3 min_aabb_b = vec3(0.0f);
            min_aabb_b.x = trans_b.position.x - col_b->width;
            min_aabb_b.y = trans_b.position.y - col_b->height;
            min_aabb_b.z = trans_b.position.z - col_b->depth;

            // Point where the box 'ends'
            vec3 max_aabb_a = vec3(0.0f);
            max_aabb_a.x = trans_a.position.x + col_a->width;
            max_aabb_a.y = trans_a.position.y + col_a->height;
            max_aabb_a.z = trans_a.position.z + col_a->depth;

            vec3 max_aabb_b = vec3(0.0f);
            max_aabb_b.x = trans_b.position.x + col_b->width;
            max_aabb_b.y = trans_b.position.y + col_b->height;
            max_aabb_b.z = trans_b.position.z + col_b->depth;

            bool intersecting = (min_aabb_a.x <= max_aabb_b.x && max_aabb_a.x >= min_aabb_b.x &&
                                 min_aabb_a.y <= max_aabb_b.y && max_aabb_a.y >= min_aabb_b.y &&
                                 min_aabb_a.z <= max_aabb_b.z && max_aabb_a.z >= min_aabb_b.z);

            if (intersecting)
            {
                vec3 overlap;
                overlap.x = max(0.0f, min(max_aabb_a.x, max_aabb_b.x) - max(min_aabb_a.x, min_aabb_b.x));
                overlap.y = max(0.0f, min(max_aabb_a.y, max_aabb_b.y) - max(min_aabb_a.y, min_aabb_b.y));
                overlap.z = max(0.0f, min(max_aabb_a.z, max_aabb_b.z) - max(min_aabb_a.z, min_aabb_b.z));

                vec3 penetration_depth = vec3(0.0f);
                if (overlap.x < overlap.y && overlap.x < overlap.z)
                {
                    if (min_aabb_a.x < min_aabb_b.x)
                    {
                        max_aabb_a.x -= overlap.x;
                        min_aabb_b.x += overlap.x;
                        penetration_depth.x = -overlap.x;
                    }
                    else
                    {
                        min_aabb_a.x += overlap.x;
                        max_aabb_b.x -= overlap.x;
                        penetration_depth.x = overlap.x;
                    }
                }

                else if (overlap.y < overlap.z)
                {
                    if (min_aabb_a.y < min_aabb_b.y)
                    {
                        max_aabb_a.y -= overlap.y;
                        min_aabb_b.y += overlap.y;
                        penetration_depth.y = -overlap.y;
                    }
                    else
                    {
                        min_aabb_a.y += overlap.y;
                        max_aabb_b.y -= overlap.y;
                        penetration_depth.y = overlap.y;
                    }
                }

                else
                {
                    if (min_aabb_a.z < min_aabb_b.z)
                    {
                        max_aabb_a.z -= overlap.z;
                        min_aabb_b.z += overlap.z;
                        penetration_depth.z = -overlap.z;
                    }
                    else
                    {
                        min_aabb_a.z += overlap.z;
                        max_aabb_b.z -= overlap.z;
                        penetration_depth.z = +overlap.z;
                    }
                }

                if (length(penetration_depth) > 0.0f)
                {
                    collision.point_a = penetration_depth;
                    collision.point_b = vec3(0.0f);
                    collision.has_collision = true;
                }
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
        vec3 delta = collision.point_a - collision.point_b;
        f32 dist = length(delta);
        vec3 normal = delta / dist;

        auto collider_a = ecs.colliders[id_a].get();
        auto collider_b = ecs.colliders[id_b].get();

        if (!collider_a->immovable)
        {
            auto trans_a = ecs.transforms[id_a].get();
            auto object_a = ecs.physics_objects[id_a].get();

            trans_a->position += normal * dist * 0.5f;
            object_a->velocity = object_a->velocity - (dot(object_a->velocity, normal) * normal);
        }

        if (!collider_b->immovable)
        {
            auto trans_b = ecs.transforms[id_b].get();
            auto object_b = ecs.physics_objects[id_b].get();

            trans_b->position -= normal * dist * 0.5f;
            object_b->velocity = object_b->velocity - (dot(object_b->velocity, normal) * normal);
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
