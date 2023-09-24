#pragma once

/**
 * @brief A FSM to manage character/objects state.
 */

#include "core/core.hpp"

#define PLAYER_STATE_IDLE    "idle"
#define PLAYER_STATE_WALKING "walking"

namespace bls
{
    class State
    {
        public:
            virtual void enter() = 0;
            virtual void update() = 0;
            virtual void exit() = 0;
    };

    class IdleState : public State
    {
        public:
            void enter() override;
            void update() override;
            void exit() override;
    };

    class WalkingState : public State
    {
        public:
            void enter() override;
            void update() override;
            void exit() override;
    };
};
