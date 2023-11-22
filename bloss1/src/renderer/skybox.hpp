#pragma once

/**
 * @brief @TODO: maybe there is a better way to design the skybox.
 */

#include "math/math.hpp"
#include "renderer/shader.hpp"

namespace bls
{
    class Skybox
    {
        public:
            virtual ~Skybox(){};

            virtual void bind(Shader &shader, u32 slot) = 0;
            virtual void draw(const mat4 &view, const mat4 &projection) = 0;

            static Skybox *create(const str &path,
                                  const u32 skybox_resolution,
                                  const u32 irradiance_resolution,
                                  const u32 brdf_resolution,
                                  const u32 prefilter_resolution,
                                  const u32 max_mip_levels);
    };
};  // namespace bls
