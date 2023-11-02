#include "ecs/ecs.hpp"
#include "ecs/entities.hpp"

namespace bls
{
    const f32 OPHANIM_MAX_HP = 1000;

    void shoot_player(ECS &ecs);

    void ophanim_controller_system(ECS &ecs, f32 dt)
    {
        str ophanim_state = OPHANIM_STATE_IDLE;

        if (ecs.hitpoints[1] < OPHANIM_MAX_HP)
        {
            ophanim_state = OPHANIM_STATE_ALERT;

            const auto &player_transform = ecs.transforms[0];
            const auto &ophanim_transform = ecs.transforms[1];

            // Rotates towards player @TODO: fix alignment
            auto rotationMatrix =
                inverse(look_at(ophanim_transform->position, player_transform->position, {0.0f, 1.0f, 0.0f}));
            auto aligned_rot = degrees(eulerAngles(quat_cast(rotationMatrix)));
            ophanim_transform->rotation = aligned_rot + vec3(0.0f, -90.0f, 0.0f);  // Compensate for model rotation

            const auto &timer = ecs.timers[1];
            timer->time += dt;
            if (timer->time >= 1.0f)
            {
                shoot_player(ecs);
                timer->time = 0.0f;
            }
        }

        update_state_machine(ecs, 1, ophanim_state, dt);
    }

    void shoot_player(ECS &ecs)
    {
        auto &ophanim_transform = ecs.transforms[1];

        // Calculate target direction vectors without vertical influence
        vec3 front = {cos(radians(ophanim_transform->rotation.y)) * cos(radians(ophanim_transform->rotation.x)),
                      sin(radians(ophanim_transform->rotation.x)),
                      sin(radians(ophanim_transform->rotation.y)) * cos(radians(ophanim_transform->rotation.x))};
        front = normalize(front);

        vec3 right = normalize(cross(front, {0.0f, 1.0f, 0.0f}));
        vec3 up = normalize(cross(right, front));

        Transform bullet_transform = *ophanim_transform;
        bullet_transform.position = bullet_transform.position + front * 30.0f;
        bullet_transform.scale = vec3(20.0f);

        PhysicsObject bullet_object = PhysicsObject(vec3(0.0f), vec3(10000.0f), front * 500'000.0f, 15.0f);

        bullet(ecs, bullet_transform, bullet_object, 1);
    }
};  // namespace bls
