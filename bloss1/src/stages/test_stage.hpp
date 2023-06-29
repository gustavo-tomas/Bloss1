#pragma once

/**
 * @brief Simple sandbox stage and not much else.
 */

#include "stages/stage.hpp"
#include "core/window.hpp"
#include "ecs/ecs.hpp"
#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "renderer/texture.hpp"
#include "renderer/primitives/cube.hpp"
#include "camera/controller.hpp"

namespace bls
{
    class TestStage : public Stage
    {
        public:
            TestStage(Renderer& renderer, Window& window);
            ~TestStage();

            void start() override;
            void update(f32 dt) override;

            bool is_running() override;

        private:
            std::unique_ptr<ECS> ecs;

            Renderer& renderer;
            Window& window;

            CameraController* controller;
            std::shared_ptr<Shader> tex_shader;

            bool running;
    };
};
