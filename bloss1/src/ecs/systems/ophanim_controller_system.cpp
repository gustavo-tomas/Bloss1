#include "ecs/ecs.hpp"

namespace bls
{
    const f32 OPHANIM_MAX_HP = 10000;

    void ophanim_controller_system(ECS &ecs, f32 dt)
    {
        auto &player_transform = ecs.transforms[0];
        auto &ophanim_transform = ecs.transforms[1];

        str ophanim_state = OPHANIM_STATE_IDLE;

        if (ecs.hitpoints[1] < OPHANIM_MAX_HP) ophanim_state = OPHANIM_STATE_ALERT;

        // Rotates towards player
        auto rotationMatrix =
            inverse(look_at(ophanim_transform->position, player_transform->position, {0.0f, 1.0f, 0.0f}));

        // GLM has some quirky quaternion handling
        auto aligned_rot = degrees(eulerAngles(quat_cast(rotationMatrix)));

        ophanim_transform->rotation = aligned_rot + vec3(0.0f, 180.0f, 0.0f);  // Compensate for model rotation

        update_state_machine(ecs, 1, ophanim_state, dt);
    }
};  // namespace bls
