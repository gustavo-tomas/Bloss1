#pragma once

#include "renderer/shader.hpp"
#include "renderer/primitives/quad.hpp"
#include "core/game.hpp"

namespace bls
{
    enum RenderPass
    {
        Base, Fog
    };

    class PostProcessingTexture
    {
        public:
            PostProcessingTexture(u32 width, u32 height)
            {
                this->width = width;
                this->height = height;

                // Create a fbo and a color attachment texture to render the scene
                fbo = std::unique_ptr<FrameBuffer>(FrameBuffer::create());
                fbo_texture = Texture::create(width, height,
                                              ImageFormat::RGB8,
                                              TextureParameter::Repeat, TextureParameter::Repeat,
                                              TextureParameter::Linear, TextureParameter::Linear);
                fbo->attach_texture(fbo_texture.get());
                rbo_depth = std::unique_ptr<RenderBuffer>(RenderBuffer::create(width, height, AttachmentType::Depth));

                fbo->check();
                fbo->unbind();

                quad = std::make_unique<Quad>(Game::get().get_renderer());
            }

            virtual ~PostProcessingTexture()
            {
                std::cout << "post processing texture destroyed successfully\n";
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
                fbo_texture->bind(0);
                quad->render();
            }

        protected:
            u32 width, height;

            std::shared_ptr<Shader> shader;
            std::unique_ptr<Quad> quad;
            std::unique_ptr<FrameBuffer> fbo;
            std::shared_ptr<Texture> fbo_texture;
            std::unique_ptr<RenderBuffer> rbo_depth;
    };

    class BasePass : public PostProcessingTexture
    {
        public:
            BasePass(u32 width, u32 height) : PostProcessingTexture(width, height)
            {
                shader = Shader::create("base_pass", "bloss1/assets/shaders/post/base.vs", "bloss1/assets/shaders/post/base.fs");
                shader->bind();
                shader->set_uniform1("screenTexture", 0U);
            }
    };

    class FogPass : public PostProcessingTexture
    {
        public:
            FogPass(u32 width, u32 height,
                    const vec3& fog_color, const vec2& min_max, const vec3& camera_position, Texture* g_buffer_position)
                : PostProcessingTexture(width, height),
                  fog_color(fog_color),
                  min_max(min_max),
                  camera_position(camera_position),
                  g_buffer_position(g_buffer_position)
            {
                shader = Shader::create("fog", "bloss1/assets/shaders/post/base.vs", "bloss1/assets/shaders/post/fog.fs");
                shader->bind();
                shader->set_uniform1("textures.screenTexture", 0U);
                shader->set_uniform1("textures.position", 1U);
            }

            void render()
            {
                shader->bind();

                shader->set_uniform2("fogMinMax", min_max);
                shader->set_uniform3("fogColor", fog_color);
                shader->set_uniform3("viewPos", camera_position);

                fbo_texture->bind(0);
                g_buffer_position->bind(1);

                quad->render();
            }

            vec3 fog_color;
            vec2 min_max;
            const vec3& camera_position;
            Texture* g_buffer_position;
    };

    class PostProcessingSystem
    {
        public:

            // Always add a base pass
            PostProcessingSystem(u32 width, u32 height)
            {
                add_render_pass(new BasePass(width, height));
            }

            ~PostProcessingSystem()
            {
                std::cout << "post processing system destroyed successfully\n";
            }

            void add_render_pass(PostProcessingTexture* texture)
            {
                render_passes.push_back(std::unique_ptr<PostProcessingTexture>(texture));
            }

            void begin()
            {
                render_passes.front()->bind();
            }

            void end()
            {
                render_passes.front()->unbind();
            }

            void render()
            {
                // Render all passes in the provided order
                for (u64 i = 1; i < render_passes.size(); i++)
                {
                    auto& prev_pass = render_passes[i - 1];
                    auto& curr_pass = render_passes[i];

                    curr_pass->bind();   // Bind texture for post processing
                    prev_pass->render(); // Render scene to texture
                    curr_pass->unbind(); // Unbind post processing texture
                }

                // Render final pass
                render_passes.back()->render();
            }

        private:
            std::vector<std::unique_ptr<PostProcessingTexture>> render_passes;
    };
};
