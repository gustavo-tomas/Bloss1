#pragma once

/**
 * @brief The maths library. For now, just a wrapper for glm.
 */

#include "core/core.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"
#include "math/to_str.hpp"

namespace bls
{
    typedef glm::vec4 vec4;
    typedef glm::vec3 vec3;
    typedef glm::vec2 vec2;

    typedef glm::mat4 mat4;
    typedef glm::mat3 mat3;

    typedef glm::quat quat;

    template <typename T>
    auto value_ptr(T x)
    {
        return glm::value_ptr(x);
    }

    template <typename T>
    auto clamp(T val, T min_val, T max_val)
    {
        return glm::clamp(val, min_val, max_val);
    }

    template <typename T>
    auto radians(T angle_deg)
    {
        return glm::radians(angle_deg);
    }

    template <typename T>
    auto min(T x, T y)
    {
        return glm::min(x, y);
    }

    template <typename T>
    auto max(T x, T y)
    {
        return glm::max(x, y);
    }

    template <typename T>
    auto distance(T p0, T p1)
    {
        return glm::distance(p0, p1);
    }

    template <typename T>
    auto normalize(T x)
    {
        return glm::normalize(x);
    }

    template <typename T>
    auto dot(T x, T y)
    {
        return glm::dot(x, y);
    }

    template <typename T>
    auto cross(T x, T y)
    {
        return glm::cross(x, y);
    }

    template <typename T>
    auto look_at(T eye, T target, T up)
    {
        return glm::lookAt(eye, target, up);
    }

    template <typename T>
    auto perspective(T fov, T aspect, T near, T far)
    {
        return glm::perspective(fov, aspect, near, far);
    }

    template <typename T>
    auto ortho(T left, T right, T bottom, T top)
    {
        return glm::ortho(left, right, bottom, top);
    }

    template <typename T>
    auto scale(T mat, T vec)
    {
        return glm::scale(mat, vec);
    }

    template <typename T>
    auto translate(T mat, T vec)
    {
        return glm::translate(mat, vec);
    }

    template <typename T>
    auto mix(T x, T y, T alpha)
    {
        return glm::mix(x, y, alpha);
    }

    template <typename T>
    auto to_mat4(T quat)
    {
        return glm::toMat4(quat);
    }

    template <typename T>
    auto angle_axis(f32 angle, T axis)
    {
        return glm::angleAxis(angle, axis);
    }
};  // namespace bls
