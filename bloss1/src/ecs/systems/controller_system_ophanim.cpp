#include "core/game.hpp"
#include "ecs/ecs.hpp"
#include "ecs/entities.hpp"

namespace bls
{
    void shoot_player(ECS &ecs);
    void rain_on_player(ECS &ecs);
    void cage_on_player(ECS &ecs);

    const f32 MIN_POS_Y = 50.0f;
    const f32 MAX_POS_Y = 100.0f;

    const f32 healthy_timer = 3.75f;
    const f32 injured_timer = 1.5f;

    f32 ophanim_initial_hp = -1;
    bool alerted = false;
    void ophanim_controller_system(ECS &ecs, f32 dt)
    {
        if (ophanim_initial_hp < 0) ophanim_initial_hp = ecs.hitpoints[1];

        // Update hitpoints
        if (ecs.texts.count(1)) ecs.texts[1]->text = to_str(static_cast<u32>(ecs.hitpoints[1]));

        str ophanim_state = OPHANIM_STATE_IDLE;

        if (ecs.hitpoints[1] < ophanim_initial_hp && ecs.hitpoints[0] > 0.0f)
        {
            ophanim_state = OPHANIM_STATE_ALERT;

            const auto &player_transform = ecs.transforms[0];
            const auto &ophanim_transform = ecs.transforms[1];

            if (!alerted)
            {
                auto &audio_engine = Game::get().get_audio_engine();

                audio_engine.load("ophanim_alerted_sfx",
                                  "bloss1/assets/sounds/193602__speedenza__deep-metal-impact-airy-angellic-chorus.wav");
                audio_engine.play("ophanim_alerted_sfx");

                alerted = true;
            }

            // Rotates towards player @TODO: fix alignment
            auto rotationMatrix =
                inverse(look_at(ophanim_transform->position, player_transform->position, {0.0f, 1.0f, 0.0f}));
            auto aligned_rot = degrees(eulerAngles(quat_cast(rotationMatrix)));
            ophanim_transform->rotation = aligned_rot + vec3(0.0f, -90.0f, 0.0f);  // Compensate for model rotation

            const auto &timer = ecs.timers[1];
            timer->time += dt;

            f32 cooldown_timer = healthy_timer;
            if (ecs.hitpoints[1] < ophanim_initial_hp / 2.0f) cooldown_timer = injured_timer;
            if (timer->time < cooldown_timer) return;

            auto &rand_engine = Game::get().get_random_engine();
            if (rand_engine.get_float(0, 1) < 0.5)
                rain_on_player(ecs);

            else
                cage_on_player(ecs);

            timer->time = 0.0f;
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

        // vec3 right = normalize(cross(front, {0.0f, 1.0f, 0.0f}));
        // vec3 up = normalize(cross(right, front));

        Transform bullet_transform = *ophanim_transform;
        bullet_transform.position = bullet_transform.position + front * 30.0f;
        bullet_transform.scale = vec3(20.0f);

        PhysicsObject bullet_object = PhysicsObject(vec3(0.0f), vec3(10000.0f), front * 500'000.0f, 15.0f);

        bullet(ecs, bullet_transform, bullet_object, 1, 2.0f, 15.0f, 1.0f);
    }

    void rain_on_player(ECS &ecs)
    {
        const auto &player_transform = ecs.transforms[0];
        const auto &player_vel = ecs.physics_objects[0]->velocity;
        const u16 num_of_bullets = 5;

        for (u16 i = 0; i < num_of_bullets; i++)
        {
            // Roughly predict player position
            const vec3 offset = vec3(0.0f, 120.0f, 0.0f) + normalize(player_vel) * 20.0f * static_cast<f32>(i);
            mat4 model_mat = mat4(1.0f);
            model_mat = glm::translate(model_mat, player_transform->position + offset);

            Transform bullet_transform;
            bullet_transform.position = vec3(model_mat[3]);
            bullet_transform.rotation = vec3(90.0f, 0.0f, 0.0f);
            bullet_transform.scale = vec3(20.0f);

            // Indicate bullet landing point
            vec3 indicator_offset = offset;
            indicator_offset.y = 0.0f;
            ophanim_target_indicator(ecs, 0, indicator_offset, vec3(0.0f), 2.0f);
        }
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
