#include "ecs/ecs.hpp"
#include "tools/profiler.hpp"

namespace bls
{
    void cleanup_system(ECS& ecs, f32 dt)
    {
        BLS_PROFILE_SCOPE("cleanup_system");

        auto& deletion_queue = ecs.deletion_queue;
        while (!deletion_queue.empty())
        {
            ecs.erase_entity(deletion_queue.front());
            deletion_queue.pop();
        }
    }
};  // namespace bls
