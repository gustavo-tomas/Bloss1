#pragma once

/**
 * @brief Random number generator.
 */

#include "core/core.hpp"

namespace bls
{
    class Random
    {
        public:
            Random();
            ~Random();

            f32 get_float(f32 begin = 0.0f, f32 end = 1.0f);
            i32 get_int(i32 begin = 0, i32 end = 1);

        private:
            std::mt19937 random_engine;
            std::uniform_int_distribution<std::mt19937::result_type> distribution;
    };
};  // namespace bls
