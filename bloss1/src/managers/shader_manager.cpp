#include "managers/shader_manager.hpp"
#include "renderer/opengl/shader.hpp"

namespace bls
{
    std::shared_ptr<Shader> ShaderManager::load(const str& name, const str& vertex_path, const str& fragment_path, const str& geometry_path)
    {
        if (exists(name))
            return shaders[name];

        // @TODO: for now, just opengl
        shaders[name] = std::make_shared<OpenGLShader>(vertex_path, fragment_path, geometry_path);
        return shaders[name];
    }

    std::shared_ptr<Shader> ShaderManager::get_shader(const str& name)
    {
        if (exists(name))
            return shaders[name];

        else
        {
            std::cerr << "shader '" << name << "' doesn't exist\n";
            exit(1);
        }
    }

    bool ShaderManager::exists(const str& name)
    {
        return shaders.count(name) > 0;
    }

    ShaderManager& ShaderManager::get()
    {
        static ShaderManager instance;
        return instance;
    }
};
