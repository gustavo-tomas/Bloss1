#pragma once

/**
 * @brief The entities of the ECS.
 */

#include "ecs/ecs.hpp"

namespace bls
{
    u32 empty_entity(ECS &ecs);
    u32 player(ECS &ecs, const Transform &transform);
    u32 bullet(ECS &ecs,
               const Transform &transform,
               const PhysicsObject &object,
               u32 sender_id,
               f32 damage,
               f32 explosion_radius,
               f32 explosion_duration);
    u32 ophanim_target_indicator(ECS &ecs, u32 target_id, const vec3 &offset, const vec3 &rotation, f32 duration);
    u32 ball(ECS &ecs, const Transform &transform);
    u32 vampire(ECS &ecs, const Transform &transform);
    u32 abomination(ECS &ecs, const Transform &transform);
    u32 floor(ECS &ecs, const Transform &transform);
    u32 directional_light(ECS &ecs, const Transform &transform, const DirectionalLight &light);
    u32 point_light(ECS &ecs, const Transform &transform, const PointLight &light);
    u32 text(ECS &ecs, const Transform &transform, const str &text, const vec3 &color);
    u32 background_music(
        ECS &ecs, const Transform &transform, const Sound &sound, const str &file, bool looping = false);
};  // namespace bls
