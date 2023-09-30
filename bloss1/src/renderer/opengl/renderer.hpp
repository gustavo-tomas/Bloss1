#pragma once

/**
 * @brief The renderer implementation for OpenGL.
 */

#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "renderer/buffers.hpp"
#include "renderer/shadow_map.hpp"
#include "renderer/post/post_processing.hpp"

namespace bls
{
    class OpenGLRenderer : public Renderer
    {
        public:
            ~OpenGLRenderer();

            void initialize() override;

            void set_viewport(u32 x, u32 y, u32 width, u32 height) override;
            void set_debug_mode(bool active) override;
            void set_blending(bool active) override;
            void set_face_culling(bool active) override;

            void clear_color(const vec4& color) override;
            void clear() override;
            void draw_indexed(RenderingMode mode, u32 count) override;
            void draw_arrays(RenderingMode mode, u32 count) override;

            std::map<str, std::shared_ptr<Shader>>& get_shaders() override;
            std::vector<std::pair<str, std::shared_ptr<Texture>>>& get_textures() override;
            std::unique_ptr<FrameBuffer>& get_gbuffer() override;
            std::unique_ptr<Skybox>& get_skybox() override;
            std::unique_ptr<Quad>& get_rendering_quad() override;
            std::unique_ptr<ShadowMap>& get_shadow_map() override;
            std::unique_ptr<PostProcessingSystem>& get_post_processing() override;

        private:
            std::unique_ptr<Quad> quad;
            std::unique_ptr<FrameBuffer> g_buffer;
            std::unique_ptr<RenderBuffer> render_buffer;
            std::map<str, std::shared_ptr<Shader>> shaders;

            std::vector<std::pair<str, std::shared_ptr<Texture>>> textures;

            std::unique_ptr<Skybox> skybox;
            std::unique_ptr<ShadowMap> shadow_map;
            std::unique_ptr<PostProcessingSystem> post_processing;
    };
};
