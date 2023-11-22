#pragma once

/**
 * @brief Creates, loads and deletes textures.
 */

#include "renderer/texture.hpp"

namespace bls
{
    class TextureManager
    {
        public:
            void load(const str &name, std::shared_ptr<Texture> texture);
            std::shared_ptr<Texture> get_texture(const str &name);
            bool exists(const str &name);

            static TextureManager &get();

        private:
            TextureManager(){};
            ~TextureManager(){};

            std::map<str, std::shared_ptr<Texture>> textures;
    };
};  // namespace bls
