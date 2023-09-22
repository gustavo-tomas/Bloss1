#pragma once

/**
 * @brief The interface for a texture. Each renderer must implement the methods.
 */

#include "core/core.hpp"

namespace bls
{
    enum class TextureType
    {
        None, Diffuse, Normal, Specular, Metalness, Roughness, AmbientOcclusion, Emissive
    };

    enum class ImageFormat
    {
        RGB8,
        RGBA8,
        RGB32F,
        RGBA32F
    };

    enum class TextureParameter
    {
        Repeat, ClampToEdge, Nearest, Linear
    };

    class Texture
    {
        public:
            virtual ~Texture() { };

            virtual void bind(u32 slot) = 0;

            virtual u32 get_id() = 0;
            virtual u32 get_width() = 0;
            virtual u32 get_height() = 0;
            virtual TextureType get_type() = 0;

            virtual void set_data(void* pixels) = 0;

            static std::shared_ptr<Texture> get_default(TextureType texture_type);
            static std::shared_ptr<Texture> create(u32 width, u32 height, ImageFormat format,
                                                   TextureParameter wrap_s, TextureParameter wrap_t,
                                                   TextureParameter min_filter, TextureParameter mag_filter);
            static std::shared_ptr<Texture> create(const str& name, const str& path, TextureType texture_type);
    };
};
