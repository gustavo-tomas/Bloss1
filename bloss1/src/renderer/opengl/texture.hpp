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
            OpenGLTexture(u32 width,
                          u32 height,
                          ImageFormat format,
                          TextureParameter wrap_s,
                          TextureParameter wrap_t,
                          TextureParameter min_filter,
                          TextureParameter mag_filter);
            OpenGLTexture(const str &path, TextureType texture_type);
            ~OpenGLTexture();

            void bind(u32 slot) override;

            u32 get_id() override;
            u32 get_width() override;
            u32 get_height() override;
            TextureType get_type() override;

            void set_data(void *pixels) override;

        private:
            str path;
            u32 texture_id;
            TextureType type;
            i32 width, height, num_components;
    };
};  // namespace bls
