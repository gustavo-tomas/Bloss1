#include "renderer/opengl/shader.hpp"

#include <GL/glew.h> // Include glew before glfw
#include <GLFW/glfw3.h>

namespace bls
{
    OpenGLShader::OpenGLShader(const str& vertex_path, const str& fragment_path, const str& geometry_path)
    {
        // Retrieve shader id from the manager
    }

    void OpenGLShader::bind()
    {
        glUseProgram(id);
    }

    void OpenGLShader::unbind()
    {
        glUseProgram(0);
    }

    void OpenGLShader::set_uniform1(const str& name, bool value)
    {
        glUniform1i(glGetUniformLocation(id, name.c_str()), (u32) value);
    }

    void OpenGLShader::set_uniform1(const str& name, u32 value)
    {
        glUniform1i(glGetUniformLocation(id, name.c_str()), value);
    }

    void OpenGLShader::set_uniform1(const str& name, f32 value)
    {
        glUniform1f(glGetUniformLocation(id, name.c_str()), value);
    }

    void OpenGLShader::set_uniform2(const str& name, const vec2& vector)
    {
        glUniform2f(glGetUniformLocation(id, name.c_str()), vector.x, vector.y);
    }

    void OpenGLShader::set_uniform3(const str& name, const vec3& vector)
    {
        glUniform3f(glGetUniformLocation(id, name.c_str()), vector.x, vector.y, vector.z);
    }

    void OpenGLShader::set_uniform3(const str& name, const mat3& matrix)
    {
        glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, value_ptr(matrix));
    }

    void OpenGLShader::set_uniform4(const str& name, const vec4& vector)
    {
        glUniform4f(glGetUniformLocation(id, name.c_str()), vector.x, vector.y, vector.z, vector.w);
    }

    void OpenGLShader::set_uniform4(const str& name, const mat4& matrix)
    {
        glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, value_ptr(matrix));
    }

    mat4 OpenGLShader::getUniformMatrix4(const str& name)
    {
        mat4 matrix;
        glGetUniformfv(id, glGetUniformLocation(id, name.c_str()), value_ptr(matrix));
        return matrix;
    }
};
