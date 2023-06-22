#pragma once

/**
 * @brief Simple sandbox stage and not much else.
 */

#include "stages/stage.hpp"
#include "core/window.hpp"
#include "ecs/ecs.hpp"
#include "renderer/renderer.hpp"
#include "renderer/buffers.hpp"
#include "renderer/shader.hpp"
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
            void render() override;

            bool is_running() override;

        private:
            std::unique_ptr<ECS> ecs;

            Renderer& renderer; // @TODO: temporary
            Window& window;

            CameraController* controller;
            std::shared_ptr<Shader> shader;

            VertexArray* vao;
            VertexBuffer* vbo;
            IndexBuffer* ebo;

            bool running;
    };
};
