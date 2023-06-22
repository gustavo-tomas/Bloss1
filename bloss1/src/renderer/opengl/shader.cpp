#include "renderer/opengl/shader.hpp"

#include <GL/glew.h> // Include glew before glfw
#include <GLFW/glfw3.h>

namespace bls
{
    OpenGLShader::OpenGLShader(const str& vertex_path, const str& fragment_path, const str& geometry_path)
    {
        // Create the shaders
        GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
        GLuint geometry_shader_id;

        if (geometry_path != "")
            geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER);

        str vertex_shader_code;
        str fragment_shader_code;
        str geometry_shader_code;

        std::ifstream vertex_shader_stream;
        std::ifstream fragment_shader_stream;
        std::ifstream geometry_shader_stream;

        // Read the Vertex Shader code from the file
        try
        {
            vertex_shader_stream.open(vertex_path);
            fragment_shader_stream.open(fragment_path);

            std::stringstream vertex_sstr, frag_sstr, geometry_sstr;

            vertex_sstr << vertex_shader_stream.rdbuf();
            vertex_shader_code = vertex_sstr.str();
            vertex_shader_stream.close();

            frag_sstr << fragment_shader_stream.rdbuf();
            fragment_shader_code = frag_sstr.str();
            fragment_shader_stream.close();

            if (geometry_path != "")
            {
                geometry_shader_stream.open(geometry_path);

                geometry_sstr << geometry_shader_stream.rdbuf();
                geometry_shader_code = geometry_sstr.str();
                geometry_shader_stream.close();
            }
        }

        catch (...)
        {
            std::cerr << "error when reading files '" << vertex_path << "' and '" << fragment_path << "'\n";
        }

        // Compile shaders
        compile_shader(vertex_path, vertex_shader_code, vertex_shader_id);
        compile_shader(fragment_path, fragment_shader_code, fragment_shader_id);

        if (geometry_path != "")
            compile_shader(geometry_path, geometry_shader_code, geometry_shader_id);

        // Link the program
        std::cout << "linking program\n";
        id = glCreateProgram();

        glAttachShader(id, vertex_shader_id);
        glAttachShader(id, fragment_shader_id);

        if (geometry_path != "")
            glAttachShader(id, geometry_shader_id);

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

            str error = "";
            for (auto c : error_message) { error += c; }
            std::cout << "error when linking shader: " << vertex_path << "\n";
            std::cout << "error when linking program: \n" << error << "\n";
            exit(1);
        }

        glDetachShader(id, vertex_shader_id);
        glDetachShader(id, fragment_shader_id);

        if (geometry_path != "")
            glDetachShader(id, geometry_shader_id);

        glDeleteShader(vertex_shader_id);
        glDeleteShader(fragment_shader_id);

        if (geometry_path != "")
            glDeleteShader(geometry_shader_id);

        std::cout << "shaders compiled & linked successfully\n";
    }

    void OpenGLShader::compile_shader(const str& path, const str& code, u32 ID)
    {
        GLint result = GL_FALSE;
        int log_length = 0;

        // Compile shader
        std::cout << "compiling shader: '" << path << "'\n";
        char const* sourcePtr = code.c_str();
        glShaderSource(ID, 1, &sourcePtr, NULL);
        glCompileShader(ID);

        // Check shader
        glGetShaderiv(ID, GL_COMPILE_STATUS, &result);
        glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &log_length);

        if (log_length > 0)
        {
            std::vector<char> error_message(log_length + 1);
            glGetShaderInfoLog(ID, log_length, NULL, &error_message[0]);

            str error = "";
            for (auto c : error_message) { error += c; }
            std::cout << "error when compiling '" << path << "': " << error << "\n";
        }
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

    vec3 OpenGLShader::get_uniform3(const str& name)
    {
        vec3 vector;
        glGetUniformfv(id, glGetUniformLocation(id, name.c_str()), value_ptr(vector));
        return vector;
    }

    mat4 OpenGLShader::get_uniform4(const str& name)
    {
        mat4 matrix;
        glGetUniformfv(id, glGetUniformLocation(id, name.c_str()), value_ptr(matrix));
        return matrix;
    }
};
