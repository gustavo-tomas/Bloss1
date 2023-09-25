#include "ecs/ecs.hpp"

namespace bls
{
    void projectile_system(ECS& ecs, f32 dt)
    {
        for (auto& [id, projectile] : ecs.projectiles)
        {
            auto& timer = ecs.timers[id];
            timer->time += dt;

            // Explode when projectile expires
            if (timer->time >= projectile->time_to_live)
            {
                // @TODO: explode on death
                ecs.erase_entity(id);
            }
        }
    }
};
