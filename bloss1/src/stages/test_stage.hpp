#pragma once

/**
 * @brief Simple sandbox stage and not much else.
 */

#include "stages/stage.hpp"
#include "core/window.hpp"
#include "ecs/ecs.hpp"
#include "renderer/renderer.hpp"
#include "renderer/skybox.hpp"
#include "renderer/shader.hpp"
#include "renderer/texture.hpp"
#include "renderer/font.hpp"
#include "renderer/video_player.hpp"
#include "renderer/primitives/quad.hpp"
#include "renderer/primitives/box.hpp"
#include "renderer/primitives/line.hpp"
#include "renderer/primitives/sphere.hpp"
#include "camera/controller.hpp"
#include "audio/audio_engine.hpp"

namespace bls
{
    class TestStage : public Stage
    {
        public:
            TestStage(Renderer& renderer, Window& window, AudioEngine& audio_engine);
            ~TestStage();

            void start() override;
            void update(f32 dt) override;

            bool is_running() override;

        private:
            void render_colliders(const mat4& projection, const mat4& view);

            std::unique_ptr<ECS> ecs;

            Renderer& renderer;
            Window& window;
            AudioEngine& audio_engine;

            std::unique_ptr<Quad> quad;
            std::unique_ptr<Box> collider_box;
            std::unique_ptr<Sphere> collider_sphere;
            std::vector<std::unique_ptr<Line>> axes;

            CameraController* controller;
            std::unique_ptr<VideoPlayer> video_player;
            std::shared_ptr<Shader> pbr_shader;
            std::shared_ptr<Shader> g_buffer_shader;
            std::shared_ptr<Shader> line_shader;
            std::unique_ptr<FrameBuffer> g_buffer;
            std::unique_ptr<RenderBuffer> render_buffer;

            Skybox* skybox;

            // @TODO: temporary
            std::unordered_map<str, std::shared_ptr<Texture>> textures;
            std::shared_ptr<Font> lena_font, inder_font;

            bool running;
    };
};
