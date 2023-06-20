#include "renderer/opengl/buffers.hpp"

#include <GL/glew.h> // Include glew before glfw
#include <GLFW/glfw3.h>

namespace bls
{
    // Vertex Buffer ---------------------------------------------------------------------------------------------------
    OpenGLVertexBuffer::OpenGLVertexBuffer(f32* vertices, u32 size)
    {
        glCreateBuffers(1, &VBO);
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
    OpenGLIndexBuffer::OpenGLIndexBuffer(u32* indices, u32 count)
    {
        glCreateBuffers(1, &IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(u32), indices, GL_STATIC_DRAW);
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
};
