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
                f32 cubeVertices[] =
                {
                    // positions
                    (-1.0f), 1.0f, -1.0f,
                    -1.0f, -1.0f, -1.0f,
                    1.0f, -1.0f, -1.0f,
                    1.0f, -1.0f, -1.0f,
                    1.0f,  1.0f, -1.0f,
                    -1.0f,  1.0f, -1.0f,

                    -1.0f, -1.0f,  1.0f,
                    -1.0f, -1.0f, -1.0f,
                    -1.0f,  1.0f, -1.0f,
                    -1.0f,  1.0f, -1.0f,
                    -1.0f,  1.0f,  1.0f,
                    -1.0f, -1.0f,  1.0f,

                    1.0f, -1.0f, -1.0f,
                    1.0f, -1.0f,  1.0f,
                    1.0f,  1.0f,  1.0f,
                    1.0f,  1.0f,  1.0f,
                    1.0f,  1.0f, -1.0f,
                    1.0f, -1.0f, -1.0f,

                    -1.0f, -1.0f,  1.0f,
                    -1.0f,  1.0f,  1.0f,
                    1.0f,  1.0f,  1.0f,
                    1.0f,  1.0f,  1.0f,
                    1.0f, -1.0f,  1.0f,
                    -1.0f, -1.0f,  1.0f,

                    -1.0f,  1.0f, -1.0f,
                    1.0f,  1.0f, -1.0f,
                    1.0f,  1.0f,  1.0f,
                    1.0f,  1.0f,  1.0f,
                    -1.0f,  1.0f,  1.0f,
                    -1.0f,  1.0f, -1.0f,

                    -1.0f, -1.0f, -1.0f,
                    -1.0f, -1.0f,  1.0f,
                    1.0f, -1.0f, -1.0f,
                    1.0f, -1.0f, -1.0f,
                    -1.0f, -1.0f, 1.0f,
                    1.0f, -1.0f,  1.0f
                };

                // Setup cube VAO
                vao = VertexArray::create();
                vao->bind();

                vbo = VertexBuffer::create(cubeVertices, sizeof(cubeVertices));
                vao->add_vertex_buffer(0, 3, ShaderDataType::Float, false, 3 * sizeof(float), (void*) 0);
            };

            ~Cube()
            {
                delete vao;
                delete vbo;

                std::cout << "Cube destroyed successfully\n";
            };

            void Render()
            {
                vao->bind();
                renderer.draw_arrays(36);
                vao->unbind();
            };

        private:
            Renderer& renderer;
            VertexArray* vao;
            VertexBuffer* vbo;
    };
};
