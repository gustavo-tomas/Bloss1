#pragma once

/**
 * @brief Simple sandbox stage and not much else.
 */

#include "stages/stage.hpp"
#include "ecs/ecs.hpp"
#include "renderer/video_player.hpp"

namespace bls
{
    class TestStage : public Stage
    {
        public:
            TestStage();
            ~TestStage();

            void start() override;
            void update(f32 dt) override;

            bool is_running() override;

        private:
            std::unique_ptr<ECS> ecs;
            std::unique_ptr<VideoPlayer> video_player;

            bool running;
    };
};
