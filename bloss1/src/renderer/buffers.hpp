#pragma once

/**
 * @brief Common buffers interface. Each renderer must implement the methods accordingly.
 */

#include "core/core.hpp"

namespace bls
{
    class VertexBuffer
    {
        public:
            virtual ~VertexBuffer() { };

            virtual void bind() = 0;
            virtual void unbind() = 0;
    };

    class IndexBuffer
    {
        public:
            virtual ~IndexBuffer() { };

            virtual void bind() = 0;
            virtual void unbind() = 0;
            virtual u32 get_count() = 0;
    };

    class VertexArray
    {
        public:
            virtual ~VertexArray() { }

            virtual void bind() = 0;
            virtual void unbind() = 0;
    };
};
