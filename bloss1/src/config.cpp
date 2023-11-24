#include "config.hpp"

namespace bls
{
    u32 AppStats::vertices = 0;
    f32 AppStats::framerate = 0.0f;
    f32 AppStats::ms_per_frame = 0.0f;

    std::vector<PassConfig> AppConfig::render_passes = {};
    SkyboxConfig AppConfig::skybox_config = {1024, 32, 1024, 1024, 10};
    bool AppConfig::render_colliders = true;
    bool AppConfig::tess_wireframe = false;
};  // namespace bls
