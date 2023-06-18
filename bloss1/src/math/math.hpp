#pragma once

/**
 * @brief The maths library. For now, just a wrapper for glm.
 */

// @TODO: type checking, unions, simd or smth

#include "core/core.hpp"

#include <glm/glm.hpp>

namespace bls
{
    template<typename T>
    class Vec4
    {
        public:
            Vec4(T x, T y, T z, T w)
            {
                this->x = x;
                this->y = y;
                this->z = z;
                this->w = w;
            }

            union { T x, r, s; };
            union { T y, g, t; };
            union { T z, b, p; };
            union { T w, a, q; };
    };

    typedef Vec4<f32> vec4;

    // Helper function for string conversion
    inline str to_str(const vec4& vec)
    {
        const str x = std::to_string(vec.x);
        const str y = std::to_string(vec.y);
        const str z = std::to_string(vec.z);
        const str w = std::to_string(vec.w);

        return "(" + x + ", " + y + ", " + z + ", " + w + ")";
    }
};
