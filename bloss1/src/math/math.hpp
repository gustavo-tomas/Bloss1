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

    template<typename T>
    auto clamp(T val, T min_val, T max_val)
    {
        return glm::clamp(val, min_val, max_val);
    }

    template<typename T>
    auto radians(T angle_deg)
    {
        return glm::radians(angle_deg);
    }

    template<typename T>
    auto normalize(T x)
    {
        return glm::normalize(x);
    }

    template<typename T>
    auto cross(T x, T y)
    {
        return glm::cross(x, y);
    }

    template<typename T>
    auto look_at(T eye, T target, T up)
    {
        return glm::lookAt(eye, target, up);
    }

    template<typename T>
    auto perspective(T fov, T aspect, T near, T far)
    {
        return glm::perspective(fov, aspect, near, far);
    }

    template<typename T>
    auto translate(T mat, T vec)
    {
        return glm::translate(mat, vec);
    }
};
