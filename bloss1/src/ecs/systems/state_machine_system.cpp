#include "ecs/ecs.hpp"
#include "ecs/state_machine.hpp"
#include "core/input.hpp"

namespace bls
{
    void player_state_machine_system(ECS& ecs, u32 id, f32 dt);
    void change_state(ECS& ecs, u32 id, State* new_state);

    void state_machine_system(ECS& ecs, f32 dt)
    {
        for (auto& [id, state_machine] : ecs.state_machines)
        {
            if (ecs.names[id] == "player")
                player_state_machine_system(ecs, id, dt);

            state_machine->current_state->update(ecs, id, state_machine->blend_factor, dt);
        }
    }

    void player_state_machine_system(ECS& ecs, u32 id, f32 dt)
    {
        auto& state_machine = ecs.state_machines[id];
        state_machine->blend_factor = clamp(state_machine->blend_factor + dt, 0.0f, 1.0f);

        const f32 TOLERANCE = 0.2f;

        auto left_x = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_LEFT_X);
        auto left_y = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_LEFT_Y);

        bool walking = fabs(left_x) >= TOLERANCE || fabs(left_y) >= TOLERANCE;

        if (walking)
            change_state(ecs, id, state_machine->states[PLAYER_STATE_WALKING].get());

        else
            change_state(ecs, id, state_machine->states[PLAYER_STATE_IDLE].get());
    }

    void change_state(ECS& ecs, u32 id, State* new_state)
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
