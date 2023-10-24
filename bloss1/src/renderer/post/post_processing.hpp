#pragma once

#include "renderer/shader.hpp"
#include "renderer/primitives/quad.hpp"
#include "core/game.hpp"

namespace bls
{
    class RenderPass
    {
        public:
            RenderPass(u32 width, u32 height)
            {
                this->width = width;
                this->height = height;

                // Create a fbo and a color attachment texture to render the scene
                fbo = std::unique_ptr<FrameBuffer>(FrameBuffer::create());
                screen_texture = Texture::create(width, height,
                                                 ImageFormat::RGB8,
                                                 TextureParameter::Repeat, TextureParameter::Repeat,
                                                 TextureParameter::Linear, TextureParameter::Linear);
                fbo->attach_texture(screen_texture.get());
                rbo_depth = std::unique_ptr<RenderBuffer>(RenderBuffer::create(width, height, AttachmentType::Depth));

                fbo->check();
                fbo->unbind();

                quad = std::make_unique<Quad>(Game::get().get_renderer());
            }

            virtual ~RenderPass()
            {

            }

            virtual void bind()
            {
                auto& renderer = Game::get().get_renderer();

                fbo->bind();

                renderer.clear();
                renderer.clear_color(vec4(0.0f, 0.0f, 0.0f, 1.0f));
            }

            virtual void unbind()
            {
                fbo->unbind();
            }

            virtual void render()
            {
                shader->bind();
                screen_texture->bind(0);
                quad->render();
            }

            virtual str get_name() = 0;

        protected:
            u32 width, height;

            std::shared_ptr<Shader> shader;
            std::unique_ptr<Quad> quad;
            std::unique_ptr<FrameBuffer> fbo;
            std::shared_ptr<Texture> screen_texture;
            std::unique_ptr<RenderBuffer> rbo_depth;
    };

    class BasePass : public RenderPass
    {
        public:
            BasePass(u32 width, u32 height) : RenderPass(width, height)
            {
                shader = Shader::create("base_pass", "bloss1/assets/shaders/post/base.vs", "bloss1/assets/shaders/post/base.fs");
                shader->bind();
                shader->set_uniform1("screenTexture", 0U);
            }

            str get_name() override { return "BasePass"; }
    };

    class FogPass : public RenderPass
    {
        public:
            FogPass(u32 width, u32 height,
                    const vec3& fog_color, const vec2& min_max, const vec3& camera_position, Texture* position_texture)
                : RenderPass(width, height),
                  fog_color(fog_color),
                  min_max(min_max),
                  camera_position(camera_position),
                  position_texture(position_texture)
            {
                shader = Shader::create("fog", "bloss1/assets/shaders/post/base.vs", "bloss1/assets/shaders/post/fog.fs");
                shader->bind();
                shader->set_uniform1("textures.screenTexture", 0U);
                shader->set_uniform1("textures.positionTexture", 1U);
            }

            void render()
            {
                shader->bind();

                shader->set_uniform2("fogMinMax", min_max);
                shader->set_uniform3("fogColor", fog_color);
                shader->set_uniform3("viewPos", camera_position);

                screen_texture->bind(0);
                position_texture->bind(1);

                quad->render();
            }

            str get_name() override { return "FogPass"; }

            vec3 fog_color;
            vec2 min_max;
            const vec3& camera_position;
            Texture* position_texture;
    };

    class BloomPass : public RenderPass
    {
        public:
            BloomPass(u32 width, u32 height,
                      u32 samples, f32 spread, f32 threshold, f32 amount)
                : RenderPass(width, height),
                  samples(samples), spread(spread), threshold(threshold), amount(amount)
            {
                shader = Shader::create("bloom", "bloss1/assets/shaders/post/base.vs", "bloss1/assets/shaders/post/bloom.fs");
                shader->bind();
                shader->set_uniform1("screenTexture", 0U);
            }

            void render()
            {
                shader->bind();

                shader->set_uniform1("samples", samples);
                shader->set_uniform1("spread", spread);
                shader->set_uniform1("threshold", threshold);
                shader->set_uniform1("amount", amount);

                screen_texture->bind(0);

                quad->render();
            }

            str get_name() override { return "BloomPass"; }

            u32 samples;
            f32 spread;
            f32 threshold;
            f32 amount;
    };

    class SharpenPass : public RenderPass
    {
        public:
            SharpenPass(u32 width, u32 height, f32 amount)
                : RenderPass(width, height), amount(amount)
            {
                shader = Shader::create("sharpen", "bloss1/assets/shaders/post/base.vs", "bloss1/assets/shaders/post/sharpen.fs");
                shader->bind();
                shader->set_uniform1("screenTexture", 0U);
            }

            void render()
            {
                shader->bind();
                shader->set_uniform1("amount", amount);

                screen_texture->bind(0);
                quad->render();
            }

            str get_name() override { return "SharpenPass"; }

            f32 amount;
    };

    class PosterizationPass : public RenderPass
    {
        public:
            PosterizationPass(u32 width, u32 height, f32 levels)
                : RenderPass(width, height), levels(levels)
            {
                shader = Shader::create("posterization", "bloss1/assets/shaders/post/base.vs", "bloss1/assets/shaders/post/posterization.fs");
                shader->bind();
                shader->set_uniform1("screenTexture", 0U);
            }

            void render()
            {
                shader->bind();
                shader->set_uniform1("levels", levels);

                screen_texture->bind(0);
                quad->render();
            }

            str get_name() override { return "PosterizationPass"; }

            f32 levels;
    };

    class PixelizationPass : public RenderPass
    {
        public:
            PixelizationPass(u32 width, u32 height, u32 pixel_size)
                : RenderPass(width, height), pixel_size(pixel_size)
            {
                shader = Shader::create("pixelization", "bloss1/assets/shaders/post/base.vs", "bloss1/assets/shaders/post/pixelization.fs");
                shader->bind();
                shader->set_uniform1("screenTexture", 0U);
            }

            void render()
            {
                shader->bind();
                shader->set_uniform1("pixelSize", pixel_size);

                screen_texture->bind(0);
                quad->render();
            }

            str get_name() override { return "PixelizationPass"; }

            u32 pixel_size;
    };

    class FXAAPass : public RenderPass
    {
        public:
            FXAAPass(u32 width, u32 height)
                : RenderPass(width, height)
            {
                shader = Shader::create("fxaa", "bloss1/assets/shaders/post/base.vs", "bloss1/assets/shaders/post/fxaa.fs");
                shader->bind();
                shader->set_uniform1("screenTexture", 0U);
            }

            void render()
            {
                shader->bind();
                screen_texture->bind(0);
                quad->render();
            }

            str get_name() override { return "FXAAPass"; }
    };

    class PostProcessingSystem
    {
        typedef std::pair<std::unique_ptr<RenderPass>, u32> pass_position_pair;

        public:
            // Always add a base pass
            PostProcessingSystem(u32 width, u32 height)
            {
                add_render_pass(new BasePass(width, height), 0);
            }

            ~PostProcessingSystem()
            {

            }

            void add_render_pass(RenderPass* texture, u32 position = 1)
            {
                render_passes.push_back({ std::unique_ptr<RenderPass>(texture), position });
                sort_render_passes();
            }

            void begin()
            {
                render_passes.front().first->bind();
            }

            void end()
            {
                render_passes.front().first->unbind();
                
                // Render all passes in the provided order
                for (u64 i = 1; i < render_passes.size(); i++)
                {
                    auto& [prev_pass, prev_pos] = render_passes[i - 1];
                    auto& [curr_pass, curr_pos] = render_passes[i];

                    curr_pass->bind();   // Bind texture for post processing
                    prev_pass->render(); // Render scene to texture
                    curr_pass->unbind(); // Unbind post processing texture
                }

                // Render final pass
                render_passes.back().first->render();
            }

        private:
            void sort_render_passes()
            {
                // Sort in ascending order
                std::sort(
                    render_passes.begin(),
                    render_passes.end(),
                    [](const pass_position_pair& a,  const pass_position_pair& b)
                    {
                        return a.second < b.second;    
                    }
                );

                // Update renderpass configs
                auto& editor = Game::get().get_editor();
                auto& configs = editor.app_configs;
                configs.render_passes = { };

                for (auto& [pass, position] : render_passes)
                    configs.render_passes.push_back({ position, pass->get_name() });

                editor.update_configs(configs);
            }

            std::vector<pass_position_pair> render_passes;
    };
};
