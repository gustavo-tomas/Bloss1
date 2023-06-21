#pragma once

/**
 * @brief The maths library. For now, just a wrapper for glm.
 */

// @TODO: type checking, unions, simd or smth

#include "math/mat.hpp"
#include "math/vec.hpp"
#include "math/to_str.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace bls
{
    typedef glm::vec4 vec4;
    typedef glm::vec3 vec3;
    typedef glm::vec2 vec2;

    typedef glm::mat4 mat4;
    typedef glm::mat3 mat3;

    template<typename T>
    auto value_ptr(T x)
    {
        return glm::value_ptr(x);
    }
};
