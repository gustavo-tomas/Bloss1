#pragma once

/**
 * @brief @TODO: maybe there is a better way to design the skybox.
 */

#include "renderer/shader.hpp"
#include "math/math.hpp"

namespace bls
{
    class Skybox
    {
        public:
            virtual ~Skybox() { };

            virtual void bind(Shader& shader, u32 slot) = 0;
            virtual void draw(const mat4& view, const mat4& projection) = 0;

            static Skybox* create(const str& path, u32 dimensions);
    };
};
