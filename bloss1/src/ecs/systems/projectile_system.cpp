#include "core/game.hpp"
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

                    if (explosion_timers[id].time == 0.0f)
                    {
                        // Decreases sound volume base on distance to listener
                        const auto listener_pos = ecs.transforms[0]->position;
                        const auto projectile_pos = ecs.transforms[id]->position;
                        f32 distance = glm::distance(listener_pos, projectile_pos);

                        auto &audio_engine = Game::get().get_audio_engine();

                        audio_engine.load("bullet_explosion_sfx",
                                          "bloss1/assets/sounds/387229__eflexmusic__explosion-closenear-mixed.wav");

                        // Reduces volume for ophanim projectile explosion
                        if (projectile->sender_id == 1)
                            audio_engine.play("bullet_explosion_sfx", vec3(0.0f), vec3(0.0f), 0.2f);

                        else
                            audio_engine.play_dist("bullet_explosion_sfx", vec3(0.0f), vec3(0.0f), distance, 1000.0f);
                    }
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
