#pragma once

/**
 * @brief The interface for a stage. I'll try to keep it as simple/flexible as possible.
 */

#include "core/core.hpp"

namespace bls
{
    class Stage
    {
        public:
            virtual ~Stage() { }

            virtual void start() = 0;
            virtual void update(f32 dt) = 0;

            virtual bool is_running() = 0;
    };
};
