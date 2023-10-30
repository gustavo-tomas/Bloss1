#include "config.hpp"

namespace bls
{
    u32 AppStats::vertices = 0;
    f32 AppStats::framerate = 0.0f;
    f32 AppStats::ms_per_frame = 0.0f;

    std::vector<PassConfig> AppConfig::render_passes = {};
};  // namespace bls
