#pragma once

#include "renderer/shader.hpp"
#include "renderer/primitives/quad.hpp"
#include "core/game.hpp"
#include "config.hpp"

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
        struct PostProcessingPass
        {
            u32 id;
            u32 position;
            RenderPass* render_pass;
            bool enabled;
        };

        public:
            // Always add a base pass
            PostProcessingSystem(u32 width, u32 height)
            {
                for (u32 i = 0; i < 100; i++)
                    available_ids.insert(i);

                add_pass(new BasePass(width, height), 0);
            }

            ~PostProcessingSystem()
            {
                for (auto& pass : passes)
                    delete pass.render_pass;
            }

            void add_pass(RenderPass* texture, u32 position = 1)
            {
                passes.push_back({ get_id(), position, texture, true });
                sort_render_passes();
            }

            void set_pass(u32 id, bool enabled, u32 position)
            {
                for (auto& pass : passes)
                {
                    if (pass.id == id)
                    {
                        pass.position = position;
                        pass.enabled = enabled;
                        break;
                    }
                }

                sort_render_passes();
            }

            void begin()
            {
                passes.front().render_pass->bind();
            }

            void end()
            {
                passes.front().render_pass->unbind();

                // Make a copy with only enabled passes
                std::vector<PostProcessingPass> enabled_passes;

                // Filter passes
                for (const auto& pass : passes)
                    if (pass.enabled)
                        enabled_passes.push_back(pass);

                // Render all passes in the provided order
                for (u64 i = 1; i < enabled_passes.size(); i++)
                {
                    auto& prev_pass = enabled_passes[i - 1];
                    auto& curr_pass = enabled_passes[i];

                    curr_pass.render_pass->bind();   // Bind texture for post processing
                    prev_pass.render_pass->render(); // Render scene to texture
                    curr_pass.render_pass->unbind(); // Unbind post processing texture
                }

                // Render final pass
                enabled_passes.back().render_pass->render();
            }

        private:
            void sort_render_passes()
            {
                // Sort in ascending order
                std::sort(
                    passes.begin(),
                    passes.end(),
                    [](const PostProcessingPass& a,  const PostProcessingPass& b)
                    {
                        return a.position < b.position;    
                    }
                );

                // Update renderpass configs
                auto& editor = Game::get().get_editor();
                auto& render_passes = AppConfig::render_passes;
                render_passes = { };

                for (const auto& pass : passes)
                    render_passes.push_back({ pass.id, pass.position, pass.render_pass->get_name(), pass.enabled });
            }

            u32 get_id()
            {
                if (available_ids.empty())
                    throw std::runtime_error("no available post processing ids left");

                u32 id = *available_ids.begin();
                available_ids.erase(id);

                return id;
            }

            std::set<u32> available_ids;
            std::vector<PostProcessingPass> passes;
    };
};
