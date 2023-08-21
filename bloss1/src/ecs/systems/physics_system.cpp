#include "ecs/systems.hpp"
#include "ecs/systems/physics_system.hpp"

// @TODO: finish and cleanup
namespace bls
{
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

    // @TODO: finish
    void physics_system(ECS& ecs, f32 dt)
    {
        resolve_collisions(ecs, dt);

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

            std::cout << "POS: " << transforms[id]->position.x << ", " << transforms[id]->position.y << ", " << transforms[id]->position.z << "\n";
            std::cout << "VEL: " << object->velocity.x << ", " << object->velocity.y << ", " << object->velocity.z << "\n";

            // Reset forces
            object->force = vec3(0.0f);
        }
    }

    void resolve_collisions(ECS& ecs, f32 dt)
    {
        auto& objects = ecs.physics_objects;
        auto& colliders = ecs.colliders;
        auto& transforms = ecs.transforms;
        for (auto& [id_a, collider_a] : colliders)
        {
            // Assume no collision happens
            collider_a->color = vec3(0.0f);

            for (auto& [id_b, collider_b] : colliders)
            {
                // Test only unique pairs
                if (id_a == id_b)
                    break;

                auto transform_a = transforms[id_a].get();
                auto transform_b = transforms[id_b].get();
                auto collision = test_collision(collider_a.get(), transform_a, collider_b.get(), transform_b);

                if (collision.has_collision)
                {
                    collider_a->color = { 1.0f, 0.0f, 0.0f };
                    collider_b->color = { 1.0f, 0.0f, 0.0f };
                    solve_collision(transform_a, objects[id_a].get(), collider_a.get(),
                                    transform_b, objects[id_b].get(), collider_b.get(),
                                    collision, dt);
                }
            }
        }
    }

    Collision test_collision(Collider* col_a, Transform* trans_a, Collider* col_b, Transform* trans_b)
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

    void solve_collision(Transform* trans_a, PhysicsObject* object_a, Collider* collider_a,
                         Transform* trans_b, PhysicsObject* object_b, Collider* collider_b,
                         Collision collision, f32 dt)
    {
        // Box v. Sphere
        if (collider_a->type == Collider::Box && collider_b->type == Collider::Sphere)
            return solve_collision(trans_a, object_a, static_cast<BoxCollider*>(collider_a),
                                   trans_b, object_b, static_cast<SphereCollider*>(collider_b),
                                   collision, dt);

        // Sphere v. Box
        else if (collider_a->type == Collider::Sphere && collider_b->type == Collider::Box)
            return solve_collision(trans_b, object_b, static_cast<BoxCollider*>(collider_b),
                                   trans_a, object_a, static_cast<SphereCollider*>(collider_a),
                                   collision, dt);

        // Sphere v. Sphere
        else if (collider_a->type == Collider::Sphere && collider_b->type == Collider::Sphere)
            return solve_collision(trans_a, object_a, static_cast<SphereCollider*>(collider_a),
                                   trans_b, object_b, static_cast<SphereCollider*>(collider_b),
                                   collision, dt);

        // Box v. Box
        // else if (collider_a->type == Collider::Box && collider_b->type == Collider::Box)
        //     return solve_collision(trans_a, object_a, static_cast<BoxCollider*>(collider_a),
        //                            trans_b, static_cast<BoxCollider*>(collider_b),
        //                            collision, dt);
    }

    // Sphere v. Sphere
    void solve_collision(Transform* trans_a, PhysicsObject* object_a, SphereCollider* collider_a,
                         Transform* trans_b, PhysicsObject* object_b, SphereCollider* collider_b,
                         Collision collision, f32 dt)
    {
        vec3 normal = collision.point_a - collision.point_b;
        // vec3 normal = trans_a->position - trans_b->position;
        f32 dist = glm::length(normal);

        normal /= dist;

        // Calculate the separation distance (overlap)
        f32 overlap = collider_a->radius + collider_b->radius - dist;

        // Move the spheres to resolve overlap
        trans_a->position += normal * overlap * 0.5f;
        trans_b->position -= normal * overlap * 0.5f;
    }

    // Box v. Sphere
    void solve_collision(Transform* trans_a, PhysicsObject* object_a, BoxCollider* collider_a,
                         Transform* trans_b, PhysicsObject* object_b, SphereCollider* collider_b,
                         Collision collision, f32 dt)
    {
        // Move the sphere to resolve overlap
        trans_b->position += vec3(0.0f, collision.point_a.y, 0.0f);
    }

    // Sphere v. Sphere
    Collision test_collision(SphereCollider* col_a, Transform* trans_a, SphereCollider* col_b, Transform* trans_b)
    {
        Collision collision = { };

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

    // Box v. Sphere
    Collision test_collision(BoxCollider* col_a, Transform* trans_a, SphereCollider* col_b, Transform* trans_b)
    {
        Collision collision = { };

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

    // Box v. Box
    Collision test_collision(BoxCollider* col_a, Transform* trans_a, BoxCollider* col_b, Transform* trans_b)
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
};
