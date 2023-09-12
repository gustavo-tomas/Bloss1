#pragma once

/**
 * @brief @TODO
 *
 */

#include "stages/stage.hpp"
#include "renderer/video_player.hpp"

namespace bls
{
    class MenuStage : public Stage
    {
        public:
            MenuStage();
            ~MenuStage();

            void start() override;
            void update(f32 dt) override;

        private:
            std::unique_ptr<VideoPlayer> video_player;
    };
};
