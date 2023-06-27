#pragma once

/**
 * @brief @TODO: will be refactored.
 */

#include "renderer/texture.hpp"

namespace bls
{
    class TextureManager
    {
        public:
            std::shared_ptr<Texture> load(const str& name, const str& path, TextureType texture_type);
            std::shared_ptr<Texture> get_texture(const str& name);
            bool exists(const str& name);

            static TextureManager& get();

        private:
            TextureManager() { };
            ~TextureManager() { };

            std::map<str, std::shared_ptr<Texture>> textures;
    };
};
