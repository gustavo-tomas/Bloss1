#pragma once

/**
 * @brief @TODO
 *
 */

#include "core/core.hpp"

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
};
