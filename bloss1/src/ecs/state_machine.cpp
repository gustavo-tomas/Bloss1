#include "ecs/state_machine.hpp"

namespace bls
{
    // Idle state
    // -----------------------------------------------------------------------------------------------------------------
    void IdleState::enter()
    {
        std::cout << "Entering Idle State\n";
    }

    void IdleState::update()
    {
        std::cout << "Idle State update\n";
    }

    void IdleState::exit()
    {
        std::cout << "Exiting Idle State\n";
    }

    // Walking state
    // -----------------------------------------------------------------------------------------------------------------
    void WalkingState::enter()
    {
        std::cout << "Entering Walking State\n";
    }

    void WalkingState::update()
    {
        std::cout << "Walking State update\n";
    }

    void WalkingState::exit()
    {
        std::cout << "Exiting Walking State\n";
    }
};
