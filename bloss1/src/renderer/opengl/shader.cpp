#include "renderer/opengl/shader.hpp"

#include <GL/glew.h>  // Include glew before glfw

#include "GLFW/glfw3.h"
#include "core/logger.hpp"

#define MAX_SHADER_ID 100000007

namespace bls
{
    OpenGLShader::OpenGLShader(const str &vertex_path,
                               const str &fragment_path,
                               const str &geometry_path,
                               const str &tess_ctrl_path,
                               const str &tess_eval_path)
    {
        // Create the shaders
        GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
        GLuint geometry_shader_id = MAX_SHADER_ID;
        GLuint tess_ctrl_shader_id = MAX_SHADER_ID;
        GLuint tess_eval_shader_id = MAX_SHADER_ID;

        if (geometry_path != "") geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER);
        if (tess_ctrl_path != "") tess_ctrl_shader_id = glCreateShader(GL_TESS_CONTROL_SHADER);
        if (tess_eval_path != "") tess_eval_shader_id = glCreateShader(GL_TESS_EVALUATION_SHADER);

        str vertex_shader_code, fragment_shader_code, geometry_shader_code;
        str tess_ctrl_shader_code, tess_eval_shader_code;

        // Read the Vertex Shader code from the file
        try
        {
            vertex_shader_code = get_code_from_file(vertex_path);
            fragment_shader_code = get_code_from_file(fragment_path);

            if (geometry_path != "") geometry_shader_code = get_code_from_file(geometry_path);
            if (tess_ctrl_path != "") tess_ctrl_shader_code = get_code_from_file(tess_ctrl_path);
            if (tess_eval_path != "") tess_eval_shader_code = get_code_from_file(tess_eval_path);
        }

        catch (...)
        {
            LOG_ERROR("error when reading files '%s' and '%s'", vertex_path, fragment_path);
        }

        // Compile shaders
        compile_shader(vertex_path, vertex_shader_code, vertex_shader_id);
        compile_shader(fragment_path, fragment_shader_code, fragment_shader_id);

        if (geometry_path != "" && geometry_shader_id != MAX_SHADER_ID)
            compile_shader(geometry_path, geometry_shader_code, geometry_shader_id);

        if (tess_ctrl_path != "" && tess_ctrl_shader_id != MAX_SHADER_ID)
            compile_shader(tess_ctrl_path, tess_ctrl_shader_code, tess_ctrl_shader_id);

        if (tess_eval_path != "" && tess_eval_shader_id != MAX_SHADER_ID)
            compile_shader(tess_eval_path, tess_eval_shader_code, tess_eval_shader_id);

        // Link the program
        LOG_INFO("linking program");
        id = glCreateProgram();

        glAttachShader(id, vertex_shader_id);
        glAttachShader(id, fragment_shader_id);

        if (geometry_path != "") glAttachShader(id, geometry_shader_id);
        if (tess_ctrl_path != "") glAttachShader(id, tess_ctrl_shader_id);
        if (tess_eval_path != "") glAttachShader(id, tess_eval_shader_id);

        glLinkProgram(id);

        GLint result = GL_FALSE;
        int log_length = 0;

        // Check the program
        glGetProgramiv(id, GL_LINK_STATUS, &result);
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &log_length);
        if (log_length > 0)
        {
            std::vector<char> error_message(log_length + 1);
            glGetProgramInfoLog(id, log_length, NULL, &error_message[0]);

            LOG_ERROR("linking program");
            LOG_ERROR("error when linking shader: '%s'", vertex_path.c_str());
            LOG_ERROR("error when linking program: '%s'", error_message.data());

            throw std::runtime_error("failed to link program");
        }

        glDetachShader(id, vertex_shader_id);
        glDetachShader(id, fragment_shader_id);

        if (geometry_path != "") glDetachShader(id, geometry_shader_id);
        if (tess_ctrl_path != "") glDetachShader(id, tess_ctrl_shader_id);
        if (tess_eval_path != "") glDetachShader(id, tess_eval_shader_id);

        glDeleteShader(vertex_shader_id);
        glDeleteShader(fragment_shader_id);

        if (geometry_path != "") glDeleteShader(geometry_shader_id);
        if (tess_ctrl_path != "") glDeleteShader(tess_ctrl_shader_id);
        if (tess_eval_path != "") glDeleteShader(tess_eval_shader_id);

        LOG_SUCCESS("shaders compiled & linked successfully");
    }

    void OpenGLShader::compile_shader(const str &path, const str &code, u32 ID)
    {
        GLint result = GL_FALSE;
        int log_length = 0;

        // Compile shader
        LOG_INFO("compiling shader '%s'", path.c_str());

        char const *sourcePtr = code.c_str();
        glShaderSource(ID, 1, &sourcePtr, NULL);
        glCompileShader(ID);

        // Check shader
        glGetShaderiv(ID, GL_COMPILE_STATUS, &result);
        glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &log_length);

        if (log_length > 0)
        {
            std::vector<char> error_message(log_length + 1);
            glGetShaderInfoLog(ID, log_length, NULL, &error_message[0]);

            LOG_ERROR("error when compiling '%s': %s", path.c_str(), error_message.data());
        }
    }

    str OpenGLShader::get_code_from_file(const str &path)
    {
        str code = "";
        std::stringstream sstr;
        std::ifstream stream;

        stream.open(path);
        sstr << stream.rdbuf();
        code = sstr.str();
        stream.close();

        return code;
    }

    void OpenGLShader::bind()
    {
        glUseProgram(id);
    }

    void OpenGLShader::unbind()
    {
        glUseProgram(0);
    }

    void OpenGLShader::set_uniform1(const str &name, bool value)
    {
        glUniform1i(glGetUniformLocation(id, name.c_str()), (u32)value);
    }

    void OpenGLShader::set_uniform1(const str &name, u32 value)
    {
        glUniform1i(glGetUniformLocation(id, name.c_str()), value);
    }

    void OpenGLShader::set_uniform1(const str &name, f32 value)
    {
        glUniform1f(glGetUniformLocation(id, name.c_str()), value);
    }

    void OpenGLShader::set_uniform2(const str &name, const vec2 &vector)
    {
        glUniform2f(glGetUniformLocation(id, name.c_str()), vector.x, vector.y);
    }

    void OpenGLShader::set_uniform3(const str &name, const vec3 &vector)
    {
        glUniform3f(glGetUniformLocation(id, name.c_str()), vector.x, vector.y, vector.z);
    }

    void OpenGLShader::set_uniform3(const str &name, const mat3 &matrix)
    {
        glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, value_ptr(matrix));
    }

    void OpenGLShader::set_uniform4(const str &name, const vec4 &vector)
    {
        glUniform4f(glGetUniformLocation(id, name.c_str()), vector.x, vector.y, vector.z, vector.w);
    }

    void OpenGLShader::set_uniform4(const str &name, const mat4 &matrix)
    {
        glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, value_ptr(matrix));
    }

    vec3 OpenGLShader::get_uniform3(const str &name)
    {
        vec3 vector;
        glGetUniformfv(id, glGetUniformLocation(id, name.c_str()), value_ptr(vector));
        return vector;
    }

    mat4 OpenGLShader::get_uniform4(const str &name)
    {
        mat4 matrix;
        glGetUniformfv(id, glGetUniformLocation(id, name.c_str()), value_ptr(matrix));
        return matrix;
    }
};  // namespace bls
