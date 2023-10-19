#pragma once

/**
 * @brief A FSM to manage character/objects state.
 */

#include "core/core.hpp"

#define PLAYER_STATE_IDLE     "Armature|Idle"
#define PLAYER_STATE_WALKING  "Armature|Walk"
#define PLAYER_STATE_JUMPING  "Armature|Jumping"
#define PLAYER_STATE_SHOOTING "Armature|Shooting"

#define OPHANIM_STATE_IDLE  "OphanimArmature|OphanimIdle"
#define OPHANIM_STATE_ALERT "OphanimArmature|OphanimAlert"

namespace bls
{
    class ECS;
    class SkeletalAnimation;

    void update_state_machine(ECS& ecs, u32 id, const str& state, f32 dt);
    
    class State
    {
        public:
            virtual void enter(ECS& ecs, u32 id, const str& state);
            virtual void update(ECS& ecs, u32 id, f32 blend_factor, f32 dt);
            virtual void exit();

        protected:
            SkeletalAnimation* last_animation = nullptr;
            SkeletalAnimation* curr_animation = nullptr;
            SkeletalAnimation* next_animation = nullptr;
    };

    class PlayerStateMachine : public State { };
    class OphanimStateMachine : public State { };
};
