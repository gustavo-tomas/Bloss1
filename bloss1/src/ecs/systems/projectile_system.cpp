#include "ecs/ecs.hpp"
#include "tools/profiler.hpp"

namespace bls
{
    std::map<u32, Timer> explosion_timers;

    void projectile_system(ECS &ecs, f32 dt)
    {
        BLS_PROFILE_SCOPE("projectile_system");

        for (auto &[id, projectile] : ecs.projectiles)
        {
            auto &timer = ecs.timers[id];
            timer->time += dt;

            // Explode when projectile expires
            if (timer->time >= projectile->time_to_live)
            {
                ecs.models.erase(id);
                ecs.physics_objects[id]->terminal_velocity = vec3(0.0f);

                ecs.colliders[id]->immovable = true;
                static_cast<SphereCollider *>(ecs.colliders[id].get())->radius = projectile->explosion_radius;

                if (!explosion_timers.count(id)) explosion_timers[id] = Timer();

                const auto &particle_sys = ecs.particle_systems[id];
                const auto &emitter_type = particle_sys->emitter->type;
                if (emitter_type == Emitter::EmitterType::Sphere)
                {
                    const auto &emitter = static_cast<SphereEmitter *>(ecs.particle_systems[id]->emitter.get());
                    emitter->radius = projectile->explosion_radius;

                    auto particle = emitter->get_particle();
                    particle.life_time = projectile->explosion_duration - explosion_timers[id].time;
                    emitter->set_particle(particle);
                }

                explosion_timers[id].time += dt;
            }

            // Delete bullet after explosion
            if (explosion_timers.count(id) && explosion_timers[id].time >= projectile->explosion_duration)
            {
                explosion_timers.erase(id);
                ecs.mark_for_deletion(id);
            }
        }
    }
};  // namespace bls
