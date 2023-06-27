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
            OpenGLVertexBuffer(f32* vertices, u32 size);
            ~OpenGLVertexBuffer();

            void bind() override;
            void unbind() override;

        private:
            u32 VBO;
    };

    class OpenGLIndexBuffer : public IndexBuffer
    {
        public:
            OpenGLIndexBuffer(u32* indices, u32 count);
            ~OpenGLIndexBuffer();

            void bind() override;
            void unbind() override;
            u32 get_count() override;

        private:
            u32 IBO;
            u32 count;
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
