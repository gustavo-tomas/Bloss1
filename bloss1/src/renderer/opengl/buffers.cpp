#include "renderer/opengl/buffers.hpp"

#include <GL/glew.h> // Include glew before glfw
#include <GLFW/glfw3.h>

namespace bls
{
    static GLenum convert_to_opengl_data(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::Float: return GL_FLOAT;
            case ShaderDataType::Int:   return GL_INT;
            case ShaderDataType::Bool:  return GL_BOOL;
            default: std::cerr << "unknown shader data type\n"; exit(1);
        }

        return 0;
    }

    // Vertex Buffer ---------------------------------------------------------------------------------------------------
    OpenGLVertexBuffer::OpenGLVertexBuffer(void* vertices, u32 size)
    {
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer()
    {
        glDeleteBuffers(1, &VBO);
    }

    void OpenGLVertexBuffer::bind()
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
    }

    void OpenGLVertexBuffer::unbind()
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Index Buffer ----------------------------------------------------------------------------------------------------
    OpenGLIndexBuffer::OpenGLIndexBuffer(const std::vector<u32>& indices, u32 count)
    {
        glGenBuffers(1, &IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(u32), &indices[0], GL_STATIC_DRAW);
    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer()
    {
        glDeleteBuffers(1, &IBO);
    }

    void OpenGLIndexBuffer::bind()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    }

    void OpenGLIndexBuffer::unbind()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    u32 OpenGLIndexBuffer::get_count()
    {
        return count;
    }

    // Frame Buffer ----------------------------------------------------------------------------------------------------
    OpenGLFrameBuffer::OpenGLFrameBuffer()
    {
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    }

    OpenGLFrameBuffer::~OpenGLFrameBuffer()
    {
        glDeleteFramebuffers(1, &FBO);
    }

    void OpenGLFrameBuffer::bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    }

    void OpenGLFrameBuffer::unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Vertex Array ----------------------------------------------------------------------------------------------------
    OpenGLVertexArray::OpenGLVertexArray()
    {
        glGenVertexArrays(1, &VAO);
    }

    OpenGLVertexArray::~OpenGLVertexArray()
    {
        glDeleteVertexArrays(1, &VAO);
    }

    void OpenGLVertexArray::bind()
    {
        glBindVertexArray(VAO);
    }

    void OpenGLVertexArray::unbind()
    {
        glBindVertexArray(0);
    }

    void OpenGLVertexArray::add_vertex_buffer(u32 index, i32 size, ShaderDataType type, bool normalized, i32 stride, void* pointer)
    {
        glVertexAttribPointer(index, size, convert_to_opengl_data(type), normalized, stride, pointer);
        glEnableVertexAttribArray(index);
    }
};
