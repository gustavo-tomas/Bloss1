#include "ecs/ecs.hpp"
#include "tools/profiler.hpp"

namespace bls
{
    void bullet_indicator_system(ECS& ecs, f32 dt)
    {
        BLS_PROFILE_SCOPE("target_indicator_system");

        for (const auto& [id, bullet_indicator] : ecs.bullet_indicators)
        {
            const auto& timer = ecs.timers[id];
            const auto& target_transform = ecs.transforms[bullet_indicator->target_id];

            vec3 target_pos = target_transform->position;
            target_pos.y -= target_transform->scale.y;  // Moves a bit closer to the base of the target

            // Recalculate offseted position from target
            auto model_mat = mat4(1.0f);
            model_mat = glm::translate(model_mat, target_pos);
            model_mat = glm::rotate(model_mat, bullet_indicator->rotation.x, vec3(1.0f, 0.0f, 0.0f));
            model_mat = glm::rotate(model_mat, bullet_indicator->rotation.y, vec3(0.0f, 1.0f, 0.0f));
            model_mat = glm::rotate(model_mat, bullet_indicator->rotation.z, vec3(0.0f, 0.0f, 1.0f));
            model_mat = glm::translate(model_mat, bullet_indicator->offset);

            const vec3 indicator_position = vec3(model_mat[3]);

            const auto& particle_sys = ecs.particle_systems[id];
            const auto& emitter = particle_sys->emitter;
            emitter->set_center(indicator_position);

            timer->time += dt;
            if (timer->time >= bullet_indicator->duration) ecs.mark_for_deletion(id);
        }
    }
};  // namespace bls
