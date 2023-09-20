#pragma once

/**
 * @brief The interface for a stage. I'll try to keep it as simple/flexible as possible.
 */

#include "ecs/ecs.hpp"

namespace bls
{
    class Stage
    {
        public:
            virtual ~Stage() { }

            virtual void start() = 0;
            virtual void update(f32 dt) = 0;

            std::unique_ptr<ECS> ecs;
    };
};
