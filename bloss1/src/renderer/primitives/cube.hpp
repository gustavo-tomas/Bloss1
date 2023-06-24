#pragma once

/**
 * @brief Simple test cube.
 */

#include "renderer/buffers.hpp"

namespace bls
{
    class Cube
    {
        public:
            Cube(Renderer& renderer) : renderer(renderer)
            {
                // Setup cube VAO
                vao = VertexArray::create();
                vao->bind();

                vbo = VertexBuffer::create(vertices, sizeof(vertices));
                ebo = IndexBuffer::create(indices, sizeof(indices) / sizeof(u32));

                vao->add_vertex_buffer(0, 3, ShaderDataType::Float, false, 0, (void*) 0);
            };

            ~Cube()
            {
                delete vao;
                delete vbo;
                delete ebo;

                std::cout << "cube destroyed successfully\n";
            };

            void Render()
            {
                vao->bind();
                renderer.draw_indexed(sizeof(indices) / sizeof(u32));
                vao->unbind();
            };

        private:
            Renderer& renderer;
            VertexArray* vao;
            VertexBuffer* vbo;
            IndexBuffer* ebo;

            f32 vertices[24] =
            {
                // Front face
                -1.0f, -1.0f, 1.0f, // (0)
                1.0f,  -1.0f, 1.0f, // (1)
                1.0f,   1.0f, 1.0f, // (2)
                -1.0f,  1.0f, 1.0f, // (3)

                // Back face
                -1.0f, -1.0f, -1.0f, // (4)
                1.0f,  -1.0f, -1.0f, // (5)
                1.0f,   1.0f, -1.0f, // (6)
                -1.0f,  1.0f, -1.0f  // (7)
            };

            u32 indices[36] =
            {
                // Front face
                0, 1, 2,
                2, 3, 0,

                // Right face
                1, 5, 6,
                6, 2, 1,

                // Back face
                7, 6, 5,
                5, 4, 7,

                // Left face
                4, 0, 3,
                3, 7, 4,

                // Bottom face
                4, 5, 1,
                1, 0, 4,

                // Top face
                3, 2, 6,
                6, 7, 3
            };
    };
};
