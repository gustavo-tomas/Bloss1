#pragma once

/**
 * @brief The OpenGL implementation of the skybox.
 */

#include "renderer/skybox.hpp"
#include "renderer/texture.hpp"
#include "renderer/buffers.hpp"
#include "renderer/primitives/cube.hpp"

namespace bls
{
    class OpenGLSkybox : public Skybox
    {
        public:
            OpenGLSkybox(const str& path, u32 dimensions);
            ~OpenGLSkybox();

            void bind(Shader& shader, u32 slot) override;
            void draw(const mat4& view, const mat4& projection) override;

        private:
            std::shared_ptr<Shader> hdr_to_cubemap_shader;
            std::shared_ptr<Shader> skybox_shader;
            std::shared_ptr<Shader> irradiance_shader;

            Cube* cube;

            std::shared_ptr<Texture> hdr_texture;
            u32 env_cubemap, irradiance_map;
            FrameBuffer* captureFBO;
            RenderBuffer* captureRBO;
    };
};
