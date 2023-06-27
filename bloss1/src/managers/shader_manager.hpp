#pragma once

/**
 * @brief Creates, loads and deletes shaders.
 */

#include "renderer/shader.hpp"

namespace bls
{
    class ShaderManager
    {
        public:
            std::shared_ptr<Shader> load(const str& name, const str& vertex_path, const str& fragment_path, const str& geometry_path = "");
            std::shared_ptr<Shader> get_shader(const str& name);
            bool exists(const str& name);

            static ShaderManager& get();

        private:
            ShaderManager() { }
            ~ShaderManager() { }

            std::map<str, std::shared_ptr<Shader>> shaders;
    };
};
