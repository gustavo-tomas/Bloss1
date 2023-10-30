#pragma once

/**
 * @brief Common buffers interface. Each renderer must implement the methods accordingly.
 */

#include "core/core.hpp"
#include "renderer/texture.hpp"

namespace bls
{
    enum class ShaderDataType
    {
        Float,
        Int,
        Bool
    };

    enum class AttachmentType
    {
        Depth
    };

    class VertexBuffer
    {
        public:
            virtual ~VertexBuffer(){};

            virtual void bind() = 0;
            virtual void unbind() = 0;

            static VertexBuffer *create(void *vertices, u32 size);
    };

    class IndexBuffer
    {
        public:
            virtual ~IndexBuffer(){};

            virtual void bind() = 0;
            virtual void unbind() = 0;
            virtual u32 get_count() = 0;

            static IndexBuffer *create(const std::vector<u32> &indices, u32 count);
    };

    class FrameBuffer
    {
        public:
            virtual ~FrameBuffer(){};

            virtual void bind() = 0;
            virtual void bind_read() = 0;
            virtual void bind_draw() = 0;
            virtual void unbind() = 0;
            virtual void blit(u32 width, u32 height) = 0;
            virtual void bind_and_blit(u32 width, u32 height) = 0;
            virtual void attach_texture(Texture *texture) = 0;
            virtual void draw() = 0;
            virtual bool check() = 0;
            virtual std::vector<Texture *> &get_attachments() = 0;

            static FrameBuffer *create();
    };

    class RenderBuffer
    {
        public:
            virtual ~RenderBuffer(){};

            virtual void bind() = 0;
            virtual void unbind() = 0;
            virtual u32 get_width() = 0;
            virtual u32 get_height() = 0;

            static RenderBuffer *create(u32 width, u32 height, AttachmentType type);
    };

    class VertexArray
    {
        public:
            virtual ~VertexArray()
            {
            }

            virtual void bind() = 0;
            virtual void unbind() = 0;
            virtual void add_vertex_buffer(
                u32 index, i32 size, ShaderDataType type, bool normalized, i32 stride, void *pointer) = 0;

            static VertexArray *create();
    };
};  // namespace bls
