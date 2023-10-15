#include "ecs/ecs.hpp"
#include "tools/profiler.hpp"

namespace bls
{
    std::map<u32, Timer> explosion_timers;

    void projectile_system(ECS& ecs, f32 dt)
    {
        BLS_PROFILE_SCOPE("projectile_system");

        for (auto& [id, projectile] : ecs.projectiles)
        {
            auto& timer = ecs.timers[id];
            timer->time += dt;

            // Explode when projectile expires
            if (timer->time >= projectile->time_to_live)
            {
                ecs.models.erase(id);
                ecs.physics_objects[id]->terminal_velocity = vec3(0.0f);

                ecs.colliders[id]->immovable = true;
                static_cast<SphereCollider*>(ecs.colliders[id].get())->radius = projectile->explosion_radius;

                if (!explosion_timers.count(id))
                    explosion_timers[id] = Timer();

                explosion_timers[id].time += dt;
            }

            // Delete bullet after explosion
            if (explosion_timers.count(id) && explosion_timers[id].time >= projectile->explosion_duration)
            {
                ecs.erase_entity(id);
                explosion_timers.erase(id);
            }
        }
    }
};
