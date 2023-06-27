#pragma once

/**
 * @brief The texture implementation for OpenGL.
 */

#include "renderer/texture.hpp"

namespace bls
{
    class OpenGLTexture : public Texture
    {
        public:
            OpenGLTexture(const str& path, TextureType texture_type);
            ~OpenGLTexture();

            void bind(u32 slot) override;

            u32 get_id() override;
            u32 get_width() override;
            u32 get_height() override;

        private:
            str path;
            u32 texture_id;
            i32 width, height, num_components;
    };
};
