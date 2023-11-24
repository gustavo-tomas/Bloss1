#pragma once

/**
 * @brief Configuration file for several parameters.
 */

#include "core/core.hpp"

namespace bls
{
    class RenderPass;
    struct PassConfig
    {
            u32 id;
            u32 position;
            str name;
            bool enabled;

            RenderPass *pass;
    };

    struct SkyboxConfig
    {
            u32 skybox_resolution;
            u32 irradiance_resolution;
            u32 brdf_resolution;
            u32 prefilter_resolution;
            u32 max_mip_levels;
    };

    class AppConfig
    {
        public:
            static std::vector<PassConfig> render_passes;
            static SkyboxConfig skybox_config;
            static bool render_colliders;
            static bool tess_wireframe;
    };

    class AppStats
    {
        public:
            static u32 vertices;
            static f32 framerate;
            static f32 ms_per_frame;
    };
};  // namespace bls
