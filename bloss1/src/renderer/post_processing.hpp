#pragma once

#include "renderer/shader.hpp"
#include "renderer/primitives/quad.hpp"

namespace bls
{
    class PostProcessingTexture
    {
        public:
            PostProcessingTexture(u32 width, u32 height);
            ~PostProcessingTexture();

            void bind();
            void unbind();
            void render();

        private:
            u32 width, height;

            std::shared_ptr<Shader> shader;
            std::unique_ptr<Quad> quad;
            std::unique_ptr<FrameBuffer> fbo;
            std::shared_ptr<Texture> fbo_texture;
            std::unique_ptr<RenderBuffer> rbo_depth;
    };
};
