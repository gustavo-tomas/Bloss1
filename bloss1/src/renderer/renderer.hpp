#pragma once

/**
 * @brief Interface for the renderer. The platform must implement the renderer accordingly.
 */

#include "math/math.hpp"
#include "renderer/buffers.hpp"
#include "renderer/shader.hpp"

namespace bls
{
    // Renderer backend (OpenGL, Vulkan, Metal, DirectX, ...)
    class Renderer
    {
        public:
            virtual ~Renderer() { }

            virtual void initialize() = 0;

            virtual VertexBuffer* create_vertex_buffer(f32* vertices, u32 size) = 0;
            virtual IndexBuffer* create_index_buffer(u32* indices, u32 count) = 0;
            virtual VertexArray* create_vertex_array() = 0;

            virtual void set_uniform() = 0;
            virtual void set_texture() = 0;
            virtual void set_render_target() = 0;
            virtual void set_viewport(u32 x, u32 y, u32 width, u32 height) = 0;

            virtual void clear_color(const vec4& color) = 0;
            virtual void clear() = 0;
            virtual void draw_indexed(u32 count) = 0;
    };
};
