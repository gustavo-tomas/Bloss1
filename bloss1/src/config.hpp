#pragma once

/**
 * @brief Configuration file for several parameters.
 */

#include "core/core.hpp"

namespace bls
{
    struct PassConfig
    {
        u32 id;
        u32 position;
        str name;
        bool enabled;
    };

    class AppConfig
    {
        public:
            static std::vector<PassConfig> render_passes;
    };

    class AppStats
    {
        public:
            static u32 vertices;
            static f32 framerate;
            static f32 ms_per_frame;
    };
};
