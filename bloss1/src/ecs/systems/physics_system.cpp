#include "ecs/systems.hpp"
#include "tools/profiler.hpp"

#define GRAVITY 50.0f
#define DECELERATION 10.0f
#define MIN_MASS 0.0001f
#define MAX_MASS 1'000'000'000.0f

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
    void update_physics(ECS& ecs, f32 dt);
    void hit_entity(ECS& ecs, u32 projectile_id, u32 hp_id);

    f64 accumulator = 0.0;
    void physics_system(ECS& ecs, f32 dt)
    {
        BLS_PROFILE_SCOPE("physics_system");

        // Clamp dt to a higher bound
        dt = clamp(static_cast<f64>(dt), 0.0, 0.1);

        auto& transforms = ecs.transforms;
        auto& objects = ecs.physics_objects;
        auto& colliders = ecs.colliders;

        std::map<u32, Transform*> previous_transforms;

        // Run physics integration in a fixed dt
        accumulator += dt;
        while (accumulator >= dt)
        {
            // Save previous state
            for (const auto& [id, object] : objects)
                previous_transforms[id] = transforms[id].get();

            update_physics(ecs, dt);
            accumulator -= dt;
        }

        // Interpolate previous and current state
        const f64 alpha = accumulator / dt;
        for (const auto& [id, object] : objects)
            if (!colliders[id]->immovable)
                transforms[id]->position = mix(transforms[id]->position, previous_transforms[id]->position, alpha);
    }

    void update_physics(ECS& ecs, f32 dt)
    {
        auto& transforms = ecs.transforms;
        auto& objects = ecs.physics_objects;
        auto& colliders = ecs.colliders;
        for (auto& [id, object] : objects)
        {
            object->mass = clamp(object->mass, MIN_MASS, MAX_MASS);

            // Do not apply forces to immovable ojbects
            if (!colliders[id]->immovable)
            {
                // Apply forces
                object->force += vec3(0.0f, object->mass * -GRAVITY, 0.0f);
                object->velocity += (object->force / object->mass) * dt;

                // Apply deceleration
                object->velocity.x = apply_deceleration(object->velocity.x, DECELERATION, object->mass, dt);
                object->velocity.y = apply_deceleration(object->velocity.y, DECELERATION, object->mass, dt);
                object->velocity.z = apply_deceleration(object->velocity.z, DECELERATION, object->mass, dt);

                object->velocity = clamp(object->velocity, -object->terminal_velocity, object->terminal_velocity);
                transforms[id]->position += object->velocity * dt;
            }

            // Reset forces
            object->force = vec3(0.0f);
        }

        resolve_collisions(ecs);
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
                    collider_a->color = collider_b->color = { 1.0f, 0.0f, 0.0f };

                    // Don't collide two projectiles
                    if (ecs.projectiles.count(id_a) && ecs.projectiles.count(id_b))
                        continue;

                    // Projectile collision (destroy projectile)
                    if (ecs.projectiles.count(id_a))
                        ecs.projectiles[id_a]->time_to_live = 0.0f;

                    if (ecs.projectiles.count(id_b))
                        ecs.projectiles[id_b]->time_to_live = 0.0f;

                    // Player hit
                    if ((ecs.projectiles.count(id_a) || ecs.projectiles.count(id_b)) &&
                        (ecs.names[id_a] == "player" || ecs.names[id_b] == "player"))
                    {
                        if (ecs.projectiles.count(id_a))
                            hit_entity(ecs, id_a, id_b);

                        else
                            hit_entity(ecs, id_b, id_a);
                    }

                    // Enemy hit
                    if ((ecs.projectiles.count(id_a) || ecs.projectiles.count(id_b)) &&
                        (ecs.names[id_a] == "ophanim" || ecs.names[id_b] == "ophanim"))
                    {
                        if (ecs.projectiles.count(id_a))
                            hit_entity(ecs, id_a, id_b);

                        else
                            hit_entity(ecs, id_b, id_a);
                    }

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
        if (collider_a->type == Collider::ColliderType::Sphere && collider_b->type == Collider::ColliderType::Box)
        {
            // Swap
            auto temp = collider_a;
            collider_a = collider_b;
            collider_b = temp;
        }

        // Box v. Sphere
        if (collider_a->type == Collider::ColliderType::Box && collider_b->type == Collider::ColliderType::Sphere)
        {
            auto col_a = static_cast<BoxCollider*>(collider_a);
            auto col_b = static_cast<SphereCollider*>(collider_b);

            // Point where the box 'begins'
            vec3 min_aabb = trans_a.position - col_a->dimensions;

            // Point where the box 'ends'
            vec3 max_aabb = trans_a.position + col_a->dimensions;

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
        else if (collider_a->type == Collider::ColliderType::Sphere && collider_b->type == Collider::ColliderType::Sphere)
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
        else if (collider_a->type == Collider::ColliderType::Box && collider_b->type == Collider::ColliderType::Box)
        {
            auto col_a = static_cast<BoxCollider*>(collider_a);
            auto col_b = static_cast<BoxCollider*>(collider_b);

            // Point where the box 'begins'
            vec3 min_aabb_a = trans_a.position - col_a->dimensions;
            vec3 min_aabb_b = trans_b.position - col_b->dimensions;

            // Point where the box 'ends'
            vec3 max_aabb_a = trans_a.position + col_a->dimensions;
            vec3 max_aabb_b = trans_b.position + col_b->dimensions;

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
        throw std::runtime_error("invalid collider types");
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

        auto trans_a = ecs.transforms[id_a].get();
        auto trans_b = ecs.transforms[id_b].get();

        auto displacement_a = normal * dist * 0.5f;
        auto displacement_b = normal * dist * 0.5f;

        if (!collider_a->immovable)
        {
            auto object_a = ecs.physics_objects[id_a].get();
            object_a->velocity = object_a->velocity - (dot(object_a->velocity, normal) * normal);
        }

        else
        {
            displacement_a = vec3(0.0f);
            displacement_b *= 2.0f;
        }

        if (!collider_b->immovable)
        {
            auto object_b = ecs.physics_objects[id_b].get();
            object_b->velocity = object_b->velocity - (dot(object_b->velocity, normal) * normal);
        }

        else
        {
            displacement_a *= 2.0f;
            displacement_b = vec3(0.0f);
        }

        trans_a->position += displacement_a;
        trans_b->position -= displacement_b;
    }

    f32 apply_deceleration(f32 velocity, f32 deceleration, f32 mass, f32 dt)
    {
        velocity -= velocity * (deceleration / mass) * dt;

        if (velocity > 0)
            return max(velocity, 0.0f);

        else if (velocity < 0)
            return min(velocity, 0.0f);

        return 0.0f;
    }

    void hit_entity(ECS& ecs, u32 projectile_id, u32 hp_id)
    {
        auto entity_hp = &ecs.hitpoints[hp_id];
        auto projectile = ecs.projectiles[projectile_id].get();

        *entity_hp = clamp(*entity_hp - projectile->damage, 0.0f, *entity_hp);
        projectile->explosion_duration = 0.0f;

        std::cout << "ID: " << hp_id << " ENTT HP: " << *entity_hp << "\n";
    }
};
