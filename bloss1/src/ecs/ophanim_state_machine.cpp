#include "ecs/ecs.hpp"
#include "ecs/state_machine.hpp"
#include "renderer/model.hpp"

namespace bls
{
    SkeletalAnimation* ophanim_last_animation = nullptr;
    SkeletalAnimation* ophanim_curr_animation = nullptr;
    SkeletalAnimation* ophanim_next_animation = nullptr;

    // Idle state
    // -----------------------------------------------------------------------------------------------------------------
    void OphanimIdleState::enter(ECS& ecs, u32 id)
    {
        auto model = ecs.models[id]->model;
        auto& animations = ecs.models[id]->model->animations;
        auto& animator = ecs.models[id]->model->animator;

        ophanim_curr_animation = model->animator->get_current_animation();
        ophanim_next_animation = animations["OphanimArmature|OphanimIdle"].get();

        // Blend from previous state to this state
        ophanim_last_animation = ophanim_curr_animation;

        animator->play(ophanim_next_animation);
        ophanim_curr_animation = ophanim_next_animation;
    }

    void OphanimIdleState::update(ECS& ecs, u32 id, f32 blend_factor, f32 dt)
    {
        auto& animator = ecs.models[id]->model->animator;
        animator->blend_animations(ophanim_last_animation, ophanim_curr_animation, blend_factor, dt);
    }

    void OphanimIdleState::exit()
    {

    }

    // Alert state
    // -----------------------------------------------------------------------------------------------------------------
    void OphanimAlertState::enter(ECS& ecs, u32 id)
    {
        auto model = ecs.models[id]->model;
        auto& animations = ecs.models[id]->model->animations;
        auto& animator = ecs.models[id]->model->animator;

        ophanim_curr_animation = model->animator->get_current_animation();
        ophanim_next_animation = animations["OphanimArmature|OphanimAlert"].get();

        // Blend from previous state to this state
        ophanim_last_animation = ophanim_curr_animation;

        animator->play(ophanim_next_animation);
        ophanim_curr_animation = ophanim_next_animation;
    }

    void OphanimAlertState::update(ECS& ecs, u32 id, f32 blend_factor, f32 dt)
    {
        auto& animator = ecs.models[id]->model->animator;
        animator->blend_animations(ophanim_last_animation, ophanim_curr_animation, blend_factor, dt);
    }

    void OphanimAlertState::exit()
    {

    }
};
