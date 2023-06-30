#pragma once

/**
 * @brief Common buffers interface. Each renderer must implement the methods accordingly.
 */

#include "core/core.hpp"

namespace bls
{
    enum ShaderDataType
    {
        Float, Int, Bool
    };

    class VertexBuffer
    {
        public:
            virtual ~VertexBuffer() { };

            virtual void bind() = 0;
            virtual void unbind() = 0;

            static VertexBuffer* create(void* vertices, u32 size);
    };

    class IndexBuffer
    {
        public:
            virtual ~IndexBuffer() { };

            virtual void bind() = 0;
            virtual void unbind() = 0;
            virtual u32 get_count() = 0;

            static IndexBuffer* create(const std::vector<u32>& indices, u32 count);
    };

    class FrameBuffer
    {
        public:
            virtual ~FrameBuffer() { };

            virtual void bind() = 0;
            virtual void unbind() = 0;

            static FrameBuffer* create();
    };

    class VertexArray
    {
        public:
            virtual ~VertexArray() { }

            virtual void bind() = 0;
            virtual void unbind() = 0;
            virtual void add_vertex_buffer(u32 index, i32 size, ShaderDataType type, bool normalized, i32 stride, void* pointer) = 0;

            static VertexArray* create();
    };
};
