#pragma once

/**
 * @brief A FSM to manage character/objects state.
 */

#include "core/core.hpp"

#define PLAYER_STATE_IDLE     "idle"
#define PLAYER_STATE_WALKING  "walking"
#define PLAYER_STATE_JUMPING  "jumping"
#define PLAYER_STATE_SHOOTING "shooting"

#define OPHANIM_STATE_IDLE "idle"

namespace bls
{
    class ECS;
    class State
    {
        public:
            virtual void enter(ECS& ecs, u32 id) = 0;
            virtual void update(ECS& ecs, u32 id, f32 blend_factor, f32 dt) = 0;
            virtual void exit() = 0;
    };

    // PlayerState
    // -----------------------------------------------------------------------------------------------------------------
    class PlayerIdleState : public State
    {
        public:
            void enter(ECS& ecs, u32 id) override;
            void update(ECS& ecs, u32 id, f32 blend_factor, f32 dt) override;
            void exit() override;
    };

    class PlayerWalkingState : public State
    {
        public:
            void enter(ECS& ecs, u32 id) override;
            void update(ECS& ecs, u32 id, f32 blend_factor, f32 dt) override;
            void exit() override;
    };

    class PlayerJumpingState : public State
    {
        public:
            void enter(ECS& ecs, u32 id) override;
            void update(ECS& ecs, u32 id, f32 blend_factor, f32 dt) override;
            void exit() override;
    };

    class PlayerShootingState : public State
    {
        public:
            void enter(ECS& ecs, u32 id) override;
            void update(ECS& ecs, u32 id, f32 blend_factor, f32 dt) override;
            void exit() override;
    };

    // OphanimState
    // -----------------------------------------------------------------------------------------------------------------
    class OphanimIdleState : public State
    {
        public:
            void enter(ECS& ecs, u32 id) override;
            void update(ECS& ecs, u32 id, f32 blend_factor, f32 dt) override;
            void exit() override;
    };
};
