#include "managers/shader_manager.hpp"

#include "renderer/shader.hpp"

namespace bls
{
    void ShaderManager::load(const str &name, std::shared_ptr<Shader> shader)
    {
        shaders[name] = shader;
    }

    std::shared_ptr<Shader> ShaderManager::get_shader(const str &name)
    {
        if (exists(name))
            return shaders[name];

        else
            throw std::runtime_error("shader '" + name + "' doesn't exist");
    }

    bool ShaderManager::exists(const str &name)
    {
        return shaders.count(name) > 0;
    }

    ShaderManager &ShaderManager::get()
    {
        static ShaderManager instance;
        return instance;
    }
};  // namespace bls
