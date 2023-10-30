#pragma once

/**
 * @brief The systems of the ECS. All systems should have the same function signature.
 */

#include "ecs/ecs.hpp"

namespace bls
{
    void render_system(ECS &ecs, f32 dt);
    void physics_system(ECS &ecs, f32 dt);
    void animation_system(ECS &ecs, f32 dt);
    void camera_system(ECS &ecs, f32 dt);
    void player_controller_system(ECS &ecs, f32 dt);
    void ophanim_controller_system(ECS &ecs, f32 dt);
    void sound_system(ECS &ecs, f32 dt);
    void state_machine_system(ECS &ecs, f32 dt);
    void projectile_system(ECS &ecs, f32 dt);
};  // namespace bls
