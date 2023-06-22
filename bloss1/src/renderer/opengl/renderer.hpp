#pragma once

/**
 * @brief The renderer implementation for OpenGL.
 */

#include "renderer/renderer.hpp"

namespace bls
{
    class OpenGLRenderer : public Renderer
    {
        public:
            ~OpenGLRenderer();

            void initialize() override;

            VertexBuffer* create_vertex_buffer(f32* vertices, u32 size) override;
            IndexBuffer* create_index_buffer(u32* indices, u32 count) override;
            VertexArray* create_vertex_array() override;

            void set_uniform() override;
            void set_texture() override;
            void set_render_target() override;
            void set_viewport(u32 x, u32 y, u32 width, u32 height) override;

            void clear_color(const vec4& color) override;
            void clear() override;
            void draw_indexed(u32 count) override;
    };
};
