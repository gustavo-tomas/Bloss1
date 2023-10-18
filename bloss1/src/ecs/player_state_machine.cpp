#include "ecs/ecs.hpp"
#include "ecs/state_machine.hpp"
#include "core/logger.hpp"
#include "renderer/model.hpp"

namespace bls
{
    SkeletalAnimation* last_animation = nullptr;
    SkeletalAnimation* curr_animation = nullptr;
    SkeletalAnimation* next_animation = nullptr;

    // Idle state
    // -----------------------------------------------------------------------------------------------------------------
    void PlayerIdleState::enter(ECS& ecs, u32 id)
    {
        auto model = ecs.models[id]->model;
        auto& animations = ecs.models[id]->model->animations;
        auto& animator = ecs.models[id]->model->animator;

        curr_animation = model->animator->get_current_animation();
        next_animation = animations["Armature|Idle"].get();

        // Blend from previous state to this state
        last_animation = curr_animation;

        animator->play(next_animation);
        curr_animation = next_animation;
    }

    void PlayerIdleState::update(ECS& ecs, u32 id, f32 blend_factor, f32 dt)
    {
        auto& animator = ecs.models[id]->model->animator;
        animator->blend_animations(last_animation, curr_animation, blend_factor, dt);
    }

    void PlayerIdleState::exit()
    {

    }

    // Walking state
    // -----------------------------------------------------------------------------------------------------------------
    void PlayerWalkingState::enter(ECS& ecs, u32 id)
    {
        auto model = ecs.models[id]->model;
        auto& animations = ecs.models[id]->model->animations;
        auto& animator = ecs.models[id]->model->animator;

        curr_animation = model->animator->get_current_animation();
        next_animation = animations["Armature|Walk"].get();

        // Blend from previous state to this state
        last_animation = curr_animation;

        animator->play(next_animation);
        curr_animation = next_animation;
    }

    void PlayerWalkingState::update(ECS& ecs, u32 id, f32 blend_factor, f32 dt)
    {
        auto& animator = ecs.models[id]->model->animator;
        animator->blend_animations(last_animation, curr_animation, blend_factor, dt);
    }

    void PlayerWalkingState::exit()
    {

    }

    // Jumping state
    // -----------------------------------------------------------------------------------------------------------------
    void PlayerJumpingState::enter(ECS& ecs, u32 id)
    {
        auto model = ecs.models[id]->model;
        auto& animations = ecs.models[id]->model->animations;
        auto& animator = ecs.models[id]->model->animator;

        curr_animation = model->animator->get_current_animation();
        next_animation = animations["Armature|Jumping"].get();

        // Blend from previous state to this state
        last_animation = curr_animation;

        animator->play(next_animation);
        curr_animation = next_animation;
    }

    void PlayerJumpingState::update(ECS& ecs, u32 id, f32 blend_factor, f32 dt)
    {
        auto& animator = ecs.models[id]->model->animator;
        animator->blend_animations(last_animation, curr_animation, blend_factor, dt);
    }

    void PlayerJumpingState::exit()
    {

    }

    // Shooting state
    // -----------------------------------------------------------------------------------------------------------------
    void PlayerShootingState::enter(ECS& ecs, u32 id)
    {
        auto model = ecs.models[id]->model;
        auto& animations = ecs.models[id]->model->animations;
        auto& animator = ecs.models[id]->model->animator;

        curr_animation = model->animator->get_current_animation();
        next_animation = animations["Armature|Shooting"].get();

        // Blend from previous state to this state
        last_animation = curr_animation;

        animator->play(next_animation);
        curr_animation = next_animation;
    }

    void PlayerShootingState::update(ECS& ecs, u32 id, f32 blend_factor, f32 dt)
    {
        auto& animator = ecs.models[id]->model->animator;
        animator->blend_animations(last_animation, curr_animation, blend_factor, dt);
    }

    void PlayerShootingState::exit()
    {

    }
};
