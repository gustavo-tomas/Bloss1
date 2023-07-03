#pragma once

/**
 * @brief Buffers implementation for OpenGL (not much to say rn).
 */

#include "renderer/buffers.hpp"

namespace bls
{
    class OpenGLVertexBuffer : public VertexBuffer
    {
        public:
            OpenGLVertexBuffer(void* vertices, u32 size);
            ~OpenGLVertexBuffer();

            void bind() override;
            void unbind() override;

        private:
            u32 VBO;
    };

    class OpenGLIndexBuffer : public IndexBuffer
    {
        public:
            OpenGLIndexBuffer(const std::vector<u32>& indices, u32 count);
            ~OpenGLIndexBuffer();

            void bind() override;
            void unbind() override;
            u32 get_count() override;

        private:
            u32 IBO;
            u32 count;
    };

    class OpenGLFrameBuffer : public FrameBuffer
    {
        public:
            OpenGLFrameBuffer();
            ~OpenGLFrameBuffer();

            void bind() override;
            void unbind() override;
            void attach_texture(Texture* texture) override;
            void draw() override;
            bool check() override;
            std::vector<Texture*>& get_attachments() override;

        private:
            u32 FBO;
            std::vector<Texture*> attachments;
    };

    class OpenGLRenderBuffer : public RenderBuffer
    {
        public:
            OpenGLRenderBuffer(u32 width, u32 height, AttachmentType type);
            ~OpenGLRenderBuffer();

            void bind() override;
            void unbind() override;
            u32 get_width() override;
            u32 get_height() override;

        private:
            u32 RBO;
            u32 width;
            u32 height;
    };

    class OpenGLVertexArray : public VertexArray
    {
        public:
            OpenGLVertexArray();
            ~OpenGLVertexArray();

            void bind() override;
            void unbind() override;

            void add_vertex_buffer(u32 index, i32 size, ShaderDataType type, bool normalized, i32 stride, void* pointer) override;

        private:
            u32 VAO;
    };
};
