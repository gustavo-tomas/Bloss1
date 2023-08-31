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
            std::unique_ptr<ECS> ecs;

            Renderer& renderer;
            Window& window;
            AudioEngine& audio_engine;

            std::unique_ptr<Quad> quad;

            std::unique_ptr<VideoPlayer> video_player;
            std::map<str, std::shared_ptr<Shader>> shaders;
            std::unique_ptr<FrameBuffer> g_buffer;
            std::unique_ptr<RenderBuffer> render_buffer;

            Skybox* skybox;

            // @TODO: temporary
            std::unordered_map<str, std::shared_ptr<Texture>> textures;
            std::shared_ptr<Font> lena_font, inder_font;

            bool running;
    };
};
