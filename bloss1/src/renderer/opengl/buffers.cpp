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

    void OpenGLFrameBuffer::attach_texture(Texture* texture)
    {
        attachments.push_back(texture);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachments.size(), GL_TEXTURE_2D, texture->get_id(), 0);
    }

    void OpenGLFrameBuffer::draw()
    {
        u32 color_attachments[attachments.size()];
        for (u32 i = 0; i < attachments.size(); i++)
            color_attachments[i] = { GL_COLOR_ATTACHMENT0 + i };

        glDrawBuffers(attachments.size(), color_attachments);
    }

    bool OpenGLFrameBuffer::check()
    {
        // Check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cerr << "error generating framebuffer: '" << FBO << "'\n";
            return false;
        }

        return true;
    }

    // Render Buffer ---------------------------------------------------------------------------------------------------
    OpenGLRenderBuffer::OpenGLRenderBuffer(u32 width, u32 height, AttachmentType)
    {
        glGenRenderbuffers(1, &RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);

        this->width = width;
        this->height = height;
    }

    OpenGLRenderBuffer::~OpenGLRenderBuffer()
    {
        glDeleteRenderbuffers(1, &RBO);
    }

    void OpenGLRenderBuffer::bind()
    {
        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    }

    void OpenGLRenderBuffer::unbind()
    {
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    u32 OpenGLRenderBuffer::get_width()
    {
        return width;
    }

    u32 OpenGLRenderBuffer::get_height()
    {
        return height;
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
