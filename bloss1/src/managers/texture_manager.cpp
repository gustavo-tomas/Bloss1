#include "managers/texture_manager.hpp"
#include "renderer/texture.hpp"

namespace bls
{
    void TextureManager::load(const str& name, std::shared_ptr<Texture> texture)
    {
        textures[name] = texture;
    }

    std::shared_ptr<Texture> TextureManager::get_texture(const str& name)
    {
        if (exists(name))
            return textures[name];

        else
        {
            std::cerr << "texture '" << name << "' doesn't exist\n";
            exit(1);
        }
    }

    bool TextureManager::exists(const str& name)
    {
        return textures.count(name) > 0;
    }

    TextureManager& TextureManager::get()
    {
        static TextureManager instance;
        return instance;
    }
};
