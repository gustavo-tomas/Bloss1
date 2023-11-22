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
            void load(const str &name, std::shared_ptr<Shader> shader);
            std::shared_ptr<Shader> get_shader(const str &name);
            bool exists(const str &name);

            static ShaderManager &get();

        private:
            ShaderManager()
            {
            }
            ~ShaderManager()
            {
            }

            std::map<str, std::shared_ptr<Shader>> shaders;
    };
};  // namespace bls
