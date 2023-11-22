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

    class AppConfig
    {
        public:
            static std::vector<PassConfig> render_passes;
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
