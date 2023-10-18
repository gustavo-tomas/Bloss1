#include "ecs/ecs.hpp"

namespace bls
{
    const f32 OPHANIM_MAX_HP = 10000;
    enum class OphanimState
    {
        Idle = 0x001,
        Alert = 0x002
    };

    void update_ophanim_state(ECS& ecs, u32 id, OphanimState ophanim_state, f32 dt);
    void change_ophanim_state(ECS& ecs, u32 id, State* new_state);

    void ophanim_controller_system(ECS& ecs, f32 dt)
    {
        OphanimState ophanim_state = OphanimState::Idle;

        if (ecs.hitpoints[1] < OPHANIM_MAX_HP)
            ophanim_state = OphanimState::Alert;

        update_ophanim_state(ecs, 1, ophanim_state, dt);
    }

    void update_ophanim_state(ECS& ecs, u32 id, OphanimState ophanim_state, f32 dt)
    {
        auto& state_machine = ecs.state_machines[id];
        state_machine->blend_factor = clamp(state_machine->blend_factor + dt, 0.0f, 1.0f);

        switch (ophanim_state)
        {
            case OphanimState::Idle:
                change_ophanim_state(ecs, id, state_machine->states[OPHANIM_STATE_IDLE].get());
                break;

            case OphanimState::Alert:
                change_ophanim_state(ecs, id, state_machine->states[OPHANIM_STATE_ALERT].get());
                break;

            default:
                change_ophanim_state(ecs, id, state_machine->states[OPHANIM_STATE_IDLE].get());
                break;
        }
    }

    void change_ophanim_state(ECS& ecs, u32 id, State* new_state)
    {
        auto state_machine = ecs.state_machines[id].get();
        if (state_machine->current_state == new_state)
            return;

        state_machine->blend_factor = 0.0f;
        state_machine->current_state->exit();
        state_machine->current_state = new_state;
        state_machine->current_state->enter(ecs, id);
    }
};
