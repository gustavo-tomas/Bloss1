#include "ecs/ecs.hpp"

namespace bls
{
    enum class OphanimState
    {
        Idle = 0x001
    };

    void update_ophanim_state(ECS& ecs, u32 id, OphanimState ophanim_state, f32 dt);

    void ophanim_controller_system(ECS& ecs, f32 dt)
    {
        OphanimState ophanim_state = OphanimState::Idle;
        update_ophanim_state(ecs, 1, ophanim_state, dt);
    }

    void update_ophanim_state(ECS& ecs, u32 id, OphanimState ophanim_state, f32 dt)
    {
        auto& state_machine = ecs.state_machines[id];
        state_machine->blend_factor = clamp(state_machine->blend_factor + dt, 0.0f, 1.0f);
    }
};
