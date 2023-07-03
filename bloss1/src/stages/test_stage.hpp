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
#include "renderer/primitives/quad.hpp"
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

            std::unique_ptr<Quad> quad;

            CameraController* controller;
            std::shared_ptr<Shader> pbr_shader;
            std::shared_ptr<Shader> g_buffer_shader;
            std::unique_ptr<FrameBuffer> g_buffer;
            std::unique_ptr<RenderBuffer> render_buffer;

            // @TODO: temporary
            std::shared_ptr<Texture> position_texture, normal_texture, albedo_texture, arm_texture, tbn_texture, depth_texture;

            u32 dir_light_id;

            bool running;
    };
};
