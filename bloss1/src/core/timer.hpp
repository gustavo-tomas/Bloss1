#pragma once

/**
 * @brief Simple timer to count time increments.
 */

#include "core/core.hpp"

namespace bls
{
    class Timer
    {
        public:
            Timer();
            ~Timer();

            void update(f32 dt);
            void restart();
            float get();

        private:
            f32 time;
    };
};
