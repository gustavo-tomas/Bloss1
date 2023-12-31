#pragma once

/**
 * @brief Simple quad for rendering.
 */

#include "renderer/buffers.hpp"
#include "renderer/renderer.hpp"

namespace bls
{
    class Quad
    {
        public:
            Quad(Renderer &renderer, bool flip_uvs = false) : renderer(renderer)
            {
                // Flip 'V'
                if (flip_uvs)
                    for (u32 i = 0, flip = 0; i < 4; i++, flip = !flip) vertices[(i + 1) * 4 + i] = flip;

                // Setup plane VAO
                vao = VertexArray::create();
                vao->bind();

                vbo = VertexBuffer::create(static_cast<void *>(vertices.data()), vertices.size() * sizeof(f32));
                ebo = IndexBuffer::create(indices, indices.size());

                vao->add_vertex_buffer(0, 3, ShaderDataType::Float, false, 5 * sizeof(f32), (void *)0);
                vao->add_vertex_buffer(1, 2, ShaderDataType::Float, false, 5 * sizeof(f32), (void *)(3 * sizeof(f32)));
            };

            ~Quad()
            {
                delete vao;
                delete vbo;
                delete ebo;
            };

            void render()
            {
                vao->bind();
                renderer.draw_indexed(RenderingMode::Triangles, indices.size());
                vao->unbind();
            };

        private:
            Renderer &renderer;
            VertexArray *vao;
            VertexBuffer *vbo;
            IndexBuffer *ebo;

            std::vector<f32> vertices = {
                // positions        // texture Coords
                -1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  // 0
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // 1
                1.0f,  1.0f,  0.0f, 1.0f, 1.0f,  // 2
                1.0f,  -1.0f, 0.0f, 1.0f, 0.0f   // 3
            };

            std::vector<u32> indices = {
                0,
                1,
                2,  // First triangle
                1,
                3,
                2  // Second triangle
            };
    };
};  // namespace bls
