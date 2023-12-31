#pragma once

/**
 * @brief Simple menu scene.
 */

// #include "renderer/video_player.hpp"
#include "stages/stage.hpp"

namespace bls
{
    class MenuStage : public Stage
    {
        public:
            MenuStage();
            ~MenuStage();

            void start() override;
            void update(f32 dt) override;
    };
};  // namespace bls
