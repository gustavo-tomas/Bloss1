#include "ecs/ecs.hpp"
#include "renderer/model.hpp"

namespace bls
{
    void State::enter(ECS& ecs, u32 id, const str& state)
    {
        auto& animations = ecs.models[id]->model->animations;
        auto& animator = ecs.models[id]->model->animator;

        curr_animation = animator->get_current_animation();
        next_animation = animations[state].get();


        // Blend from previous state to this state
        last_animation = curr_animation;

        animator->play(next_animation);
        curr_animation = next_animation;
    }

    void State::update(ECS& ecs, u32 id, f32 blend_factor, f32 dt)
    {
        auto& animator = ecs.models[id]->model->animator;
        animator->blend_animations(last_animation, curr_animation, blend_factor, dt);
    }

    void State::exit()
    {

    }

    void update_state_machine(ECS& ecs, u32 id, const str& state, f32 dt)
    {
        auto& state_machine = ecs.state_machines[id];
        state_machine->blend_factor = clamp(state_machine->blend_factor + dt, 0.0f, 1.0f);
        
        if (state_machine->current_state == state)
            return;

        state_machine->blend_factor = 1.0f - state_machine->blend_factor;
        state_machine->state->exit();
        state_machine->state->enter(ecs, id, state);
        state_machine->current_state = state;
    }
};
