#pragma once

/**
 * @brief The interface for a texture. Each renderer must implement the methods.
 */

#include "core/core.hpp"

namespace bls
{
    enum TextureType
    {
        Diffuse, Normal, Specular, Metalness, Roughness, AmbientOcclusion
    };

    class Texture
    {
        public:
            virtual ~Texture() { };

            virtual void bind(u32 slot) = 0;

            virtual u32 get_id() = 0;
            virtual u32 get_width() = 0;
            virtual u32 get_height() = 0;

            static std::shared_ptr<Texture> create(const str& name, const str& path, TextureType texture_type);
    };
};
