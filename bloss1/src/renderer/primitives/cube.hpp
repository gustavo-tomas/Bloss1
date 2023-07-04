#pragma once

/**
 * @brief Simple test cube (also for rendering).
 */

#include "renderer/renderer.hpp"
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

                vbo = VertexBuffer::create(static_cast<void*>(vertices.data()), vertices.size() * sizeof(f32));
                ebo = IndexBuffer::create(indices, indices.size());

                // Position
                vao->add_vertex_buffer(0, 3, ShaderDataType::Float, false, 8 * sizeof(f32), (void*) 0);

                // Normal
                vao->add_vertex_buffer(1, 3, ShaderDataType::Float, false, 8 * sizeof(f32), (void*) (3 * sizeof(f32)));

                // Texture coordinates
                vao->add_vertex_buffer(2, 2, ShaderDataType::Float, false, 8 * sizeof(f32), (void*) (6 * sizeof(f32)));
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
                // Front face
                //Position           // Normals         // Texture coordinates
                -1.0f, -1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // (0)
                1.0f,  -1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // (1)
                1.0f,   1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // (2)
                -1.0f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // (3)

                // Back face
                //Position            // Normals          // Texture coordinates
                -1.0f, -1.0f, -1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, // (4)
                1.0f,  -1.0f, -1.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f, // (5)
                1.0f,   1.0f, -1.0f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f, // (6)
                -1.0f,  1.0f, -1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f  // (7)
            };

            std::vector<u32> indices =
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
