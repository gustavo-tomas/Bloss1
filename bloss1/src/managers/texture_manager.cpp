#include "managers/texture_manager.hpp"
#include "renderer/opengl/texture.hpp"

namespace bls
{
    std::shared_ptr<Texture> TextureManager::load(const str& name, const str& path, TextureType texture_type)
    {
        if (exists(name))
            return textures[name];

        // @TODO: for now, just opengl
        textures[name] = std::make_shared<OpenGLTexture>(path, texture_type);
        return textures[name];
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
