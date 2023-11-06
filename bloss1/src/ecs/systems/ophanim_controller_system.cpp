#include "ecs/ecs.hpp"
#include "ecs/entities.hpp"

namespace bls
{
    void shoot_player(ECS &ecs);
    void rain_on_player(ECS &ecs);
    void cage_on_player(ECS &ecs);

    f32 ophanim_initial_hp = -1;
    void ophanim_controller_system(ECS &ecs, f32 dt)
    {
        if (ophanim_initial_hp < 0) ophanim_initial_hp = ecs.hitpoints[1];

        str ophanim_state = OPHANIM_STATE_IDLE;

        if (ecs.hitpoints[1] < ophanim_initial_hp)
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
            if (timer->time >= 3.75f)
            {
                // shoot_player(ecs);
                // rain_on_player(ecs);
                cage_on_player(ecs);
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

        bullet(ecs, bullet_transform, bullet_object, 1, 2.0f, 15.0f, 1.0f);
    }

    void rain_on_player(ECS &ecs)
    {
        const auto &player_transform = ecs.transforms[0];

        Transform bullet_transform = *player_transform;
        bullet_transform.position = bullet_transform.position + vec3(0.0f, 100.0f, 0.0f);
        bullet_transform.rotation = vec3(90.0f, 0.0f, 0.0f);
        bullet_transform.scale = vec3(20.0f);

        PhysicsObject bullet_object =
            PhysicsObject(vec3(0.0f), vec3(10000.0f), vec3(0.0f, -1.0f, 0.0f) * 200'000.0f, 15.0f);

        bullet(ecs, bullet_transform, bullet_object, 1, 2.0f, 15.0f, 1.0f);
    }

    void cage_on_player(ECS &ecs)
    {
        const auto &player_transform = ecs.transforms[0];
        const vec3 offset = vec3(25.0f, 120.0f, 25.0f);
        const u16 num_of_bullets = 7;

        for (u16 i = 0; i < num_of_bullets; i++)
        {
            mat4 model_mat = mat4(1.0f);

            const f32 angle = glm::radians(360.0f / static_cast<f32>(num_of_bullets)) * static_cast<f32>(i);
            model_mat = glm::translate(model_mat, player_transform->position);
            model_mat = glm::rotate(model_mat, angle, vec3(0.0f, 1.0f, 0.0f));
            model_mat = glm::translate(model_mat, offset);

            Transform bullet_transform;
            bullet_transform.position = vec3(model_mat[3]);
            bullet_transform.rotation = vec3(90.0f, 0.0f, 0.0f);
            bullet_transform.scale = vec3(20.0f);

            // Indicate bullet landing point
            vec3 indicator_offset = offset;
            indicator_offset.y = 0.0f;
            ophanim_target_indicator(ecs, 0, indicator_offset, vec3(0.0f, angle, 0.0f), 2.0f);

            // Bullet fired in the indicator system
            // auto bullet_object = PhysicsObject(vec3(0.0f), vec3(10000.0f), vec3(0.0f, -1.0f, 0.0f) *
            // 200'000.0f, 15.0f); bullet(ecs, bullet_transform, bullet_object, 1, 2.0f, 10.0f, 1.0f);
        }
    }
};  // namespace bls
