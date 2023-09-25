#pragma once

/**
 * @brief A FSM to manage character/objects state.
 */

#include "core/core.hpp"

#define PLAYER_STATE_IDLE    "idle"
#define PLAYER_STATE_WALKING "walking"

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
};
