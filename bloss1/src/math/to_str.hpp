#pragma once

/**
 * @brief @TODO
 *
 */

#include "math/vec.hpp"
#include "glm/glm/gtx/string_cast.hpp"

namespace bls
{
    // Helper function for string conversion
    inline str to_str(const glm::vec3& vec)
    {
        const str vec_str = glm::to_string(vec);

        return vec_str;
    }

    template<typename T>
    inline str to_str(T num)
    {
        const str num_str = std::to_string(num);

        return num_str;
    }
};
