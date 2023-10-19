#include "ecs/ecs.hpp"

namespace bls
{
    const f32 OPHANIM_MAX_HP = 10000;

    void ophanim_controller_system(ECS& ecs, f32 dt)
    {
        str ophanim_state = OPHANIM_STATE_IDLE;

        if (ecs.hitpoints[1] < OPHANIM_MAX_HP)
            ophanim_state = OPHANIM_STATE_ALERT;

        update_state_machine(ecs, 1, ophanim_state, dt);
    }
};
