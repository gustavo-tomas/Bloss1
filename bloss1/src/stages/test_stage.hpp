#pragma once

/**
 * @brief Simple sandbox stage and not much else.
 */

#include "stages/stage.hpp"

namespace bls
{
    class TestStage : public Stage
    {
        public:
            TestStage();
            ~TestStage();

            void start() override;
            void update(f32 dt) override;
    };
};  // namespace bls
