#include "ecs/ecs.hpp"
#include "ecs/state_machine.hpp"
#include "tools/profiler.hpp"

namespace bls
{
    void state_machine_system(ECS& ecs, f32 dt)
    {
        BLS_PROFILE_SCOPE("state_machine_system");

        for (auto& [id, state_machine] : ecs.state_machines)
            state_machine->current_state->update(ecs, id, state_machine->blend_factor, dt);
    }
};
