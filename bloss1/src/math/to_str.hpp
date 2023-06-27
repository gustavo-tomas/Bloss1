#pragma once

/**
 * @brief @TODO
 *
 */

#include "math/vec.hpp"

namespace bls
{
    // Helper function for string conversion
    inline str to_str(const Vec4<f32>& vec)
    {
        const str x = std::to_string(vec.x);
        const str y = std::to_string(vec.y);
        const str z = std::to_string(vec.z);
        const str w = std::to_string(vec.w);

        return "(" + x + ", " + y + ", " + z + ", " + w + ")";
    }
};
