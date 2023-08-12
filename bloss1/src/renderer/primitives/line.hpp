#pragma once

/**
 * @brief Simple test line (also for rendering).
 */

#include "renderer/renderer.hpp"
#include "renderer/buffers.hpp"

namespace bls
{
    class Line
    {
        public:
            Line(Renderer& renderer, const vec3& start, const vec3& end) : renderer(renderer)
            {
                // Set line start and end points
                vertices =
                {
                    start.x, start.y, start.z,
                    end.x, end.y, end.z
                };

                // Setup plane VAO
                vao = VertexArray::create();
                vao->bind();

                vbo = VertexBuffer::create(static_cast<void*>(vertices.data()), vertices.size() * sizeof(f32));

                vao->add_vertex_buffer(0, 3, ShaderDataType::Float, false, 3 * sizeof(f32), (void*) 0);
            };

            ~Line()
            {
                delete vao;
                delete vbo;

                std::cout << "line destroyed successfully\n";
            };

            void render()
            {
                vao->bind();
                renderer.draw_lines(2);
                vao->unbind();
            };

        private:
            Renderer& renderer;
            VertexArray* vao;
            VertexBuffer* vbo;

            // Will be filled in the constructor
            std::vector<f32> vertices =
            {
                0.0f, 0.0f, 0.0f,
                0.0f, -10.0f, 0.0f
            };
    };
};
