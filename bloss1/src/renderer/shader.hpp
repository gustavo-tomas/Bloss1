#pragma once

/**
 * @brief The interface for the shader. Each renderer must implement the methods.
 */

#include "math/math.hpp"

namespace bls
{
    class Shader
    {
        public:
            virtual ~Shader(){};

            // Use/activate the shader
            virtual void bind() = 0;
            virtual void unbind() = 0;

            // Uniform functions
            virtual void set_uniform1(const str &name, bool value) = 0;
            virtual void set_uniform1(const str &name, u32 value) = 0;
            virtual void set_uniform1(const str &name, f32 value) = 0;

            virtual void set_uniform2(const str &name, const vec2 &vector) = 0;

            virtual void set_uniform3(const str &name, const vec3 &vector) = 0;
            virtual void set_uniform3(const str &name, const mat3 &matrix) = 0;

            virtual void set_uniform4(const str &name, const vec4 &vector) = 0;
            virtual void set_uniform4(const str &name, const mat4 &matrix) = 0;

            virtual vec3 get_uniform3(const str &name) = 0;
            virtual mat4 get_uniform4(const str &name) = 0;

            static std::shared_ptr<Shader> create(const str &name,
                                                  const str &vertex_path,
                                                  const str &fragment_path,
                                                  const str &geometry_path = "");
    };
};  // namespace bls
