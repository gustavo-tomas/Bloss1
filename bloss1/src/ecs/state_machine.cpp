#include "ecs/ecs.hpp"
#include "renderer/model.hpp"

namespace bls
{
    void State::enter(ECS& ecs, u32 id, const str& state)
    {
        auto& animations = ecs.models[id]->model->animations;
        auto& animator = ecs.models[id]->model->animator;

        // Blend from previous state to this state
        last_animation = animator->get_current_animation();
        curr_animation = animations[state].get();

        auto blend_factor = animator->get_blend_factor();

        // Idle/Walking transitions
        if ((curr_animation->get_name() == PLAYER_STATE_IDLE || curr_animation->get_name() == PLAYER_STATE_WALKING) &&
            (last_animation->get_name() == PLAYER_STATE_IDLE || last_animation->get_name() == PLAYER_STATE_WALKING))
            animator->crossfade_from(last_animation, 1.0f - blend_factor, true);

        // Shooting transition
        else if (curr_animation->get_name() == PLAYER_STATE_SHOOTING || last_animation->get_name() == PLAYER_STATE_SHOOTING)
            animator->crossfade_from(last_animation, 1.0f, false);

        else
            animator->crossfade_from(last_animation, 1.0f - blend_factor, true);

        animator->play(curr_animation);
    }

    void State::update(ECS& ecs, u32 id, f32 dt)
    {
        auto& animator = ecs.models[id]->model->animator;
        animator->update_blended(dt);
    }

    void State::exit()
    {

    }

    void update_state_machine(ECS& ecs, u32 id, const str& state, f32 dt)
    {
        const auto& state_machine = ecs.state_machines[id];
        if (state_machine->current_state == state)
            return;

        state_machine->state->exit();
        state_machine->state->enter(ecs, id, state);
        state_machine->current_state = state;
    }
};
