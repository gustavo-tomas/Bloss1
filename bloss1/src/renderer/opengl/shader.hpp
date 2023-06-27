#pragma once

/**
 * @brief The shader implementation for OpenGL.
 */

#include "renderer/shader.hpp"

namespace bls
{
    class OpenGLShader : public Shader
    {
        public:
            OpenGLShader(const str& vertex_path, const str& fragment_path, const str& geometry_path = "");

            // Use/activate the shader
            void bind() override;
            void unbind() override;

            // Uniform functions
            void set_uniform1(const str& name, bool value) override;
            void set_uniform1(const str& name, u32 value) override;
            void set_uniform1(const str& name, f32 value) override;

            void set_uniform2(const str& name, const vec2& vector) override;

            void set_uniform3(const str& name, const vec3& vector) override;
            void set_uniform3(const str& name, const mat3& matrix) override;

            void set_uniform4(const str& name, const vec4& vector) override;
            void set_uniform4(const str& name, const mat4& matrix) override;

            vec3 get_uniform3(const str& name) override;
            mat4 get_uniform4(const str& name) override;

        private:
            void compile_shader(const str& path, const str& code, u32 ID);

            u32 id;
    };
};
