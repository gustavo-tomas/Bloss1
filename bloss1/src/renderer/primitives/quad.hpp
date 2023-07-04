#pragma once

/**
 * @brief Simple quad for rendering.
 */

#include "renderer/renderer.hpp"
#include "renderer/buffers.hpp"

namespace bls
{
    class Quad
    {
        public:
            Quad(Renderer& renderer) : renderer(renderer)
            {
                // Setup plane VAO
                vao = VertexArray::create();
                vao->bind();

                vbo = VertexBuffer::create(static_cast<void*>(vertices.data()), vertices.size() * sizeof(f32));
                ebo = IndexBuffer::create(indices, indices.size());

                vao->add_vertex_buffer(0, 3, ShaderDataType::Float, false, 5 * sizeof(f32), (void*) 0);
                vao->add_vertex_buffer(1, 2, ShaderDataType::Float, false, 5 * sizeof(f32), (void*)(3 * sizeof(f32)));
            };

            ~Quad()
            {
                delete vao;
                delete vbo;
                delete ebo;

                std::cout << "quad destroyed successfully\n";
            };

            void render()
            {
                vao->bind();
                renderer.draw_indexed(indices.size());
                vao->unbind();
            };

        private:
            Renderer& renderer;
            VertexArray* vao;
            VertexBuffer* vbo;
            IndexBuffer* ebo;

            std::vector<f32> vertices =
            {
                // positions          // texture Coords
                (-1.0f),  1.0f, 0.0f, 0.0f, 1.0f, // 0
                (-1.0f), -1.0f, 0.0f, 0.0f, 0.0f, // 1
                ( 1.0f),  1.0f, 0.0f, 1.0f, 1.0f, // 2
                ( 1.0f), -1.0f, 0.0f, 1.0f, 0.0f  // 3
            };

            std::vector<u32> indices =
            {
                0, 1, 2, // First triangle
                1, 3, 2  // Second triangle
            };
    };
};
