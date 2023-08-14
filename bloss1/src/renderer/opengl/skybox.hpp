#pragma once

/**
 * @brief The OpenGL implementation of the skybox.
 */

#include "renderer/skybox.hpp"
#include "renderer/texture.hpp"
#include "renderer/buffers.hpp"
#include "renderer/primitives/box.hpp"
#include "renderer/primitives/quad.hpp"

namespace bls
{
    class OpenGLSkybox : public Skybox
    {
        public:
            OpenGLSkybox(const str& path,
                         const u32 skybox_resolution,
                         const u32 irradiance_resolution,
                         const u32 brdf_resolution,
                         const u32 prefilter_resolution,
                         const u32 max_mip_levels);
            ~OpenGLSkybox();

            void bind(Shader& shader, u32 slot) override;
            void draw(const mat4& view, const mat4& projection) override;

        private:
            std::shared_ptr<Shader> hdr_to_cubemap_shader;
            std::shared_ptr<Shader> skybox_shader;
            std::shared_ptr<Shader> irradiance_shader, prefilter_shader, brdf_shader;

            Box* cube;
            Quad* quad;

            std::shared_ptr<Texture> hdr_texture;
            u32 env_cubemap, irradiance_map, prefilter_map, brdf_texture;
            FrameBuffer* captureFBO;
            RenderBuffer* captureRBO;
    };
};
