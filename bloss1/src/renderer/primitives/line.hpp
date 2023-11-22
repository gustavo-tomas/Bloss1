#pragma once

/**
 * @brief Simple line primitive with start and end points.
 */

#include "renderer/buffers.hpp"
#include "renderer/renderer.hpp"

namespace bls
{
    class Line
    {
        public:
            Line(Renderer &renderer, const vec3 &start, const vec3 &end) : renderer(renderer)
            {
                // Set line start and end points
                vertices = {start.x, start.y, start.z, end.x, end.y, end.z};

                // Setup VAO
                vao = VertexArray::create();
                vao->bind();

                vbo = VertexBuffer::create(static_cast<void *>(vertices.data()), vertices.size() * sizeof(f32));

                vao->add_vertex_buffer(0, 3, ShaderDataType::Float, false, 3 * sizeof(f32), (void *)0);
            };

            ~Line()
            {
                delete vao;
                delete vbo;
            };

            void render()
            {
                vao->bind();
                renderer.draw_arrays(RenderingMode::Lines, 2);
                vao->unbind();
            };

        private:
            Renderer &renderer;
            VertexArray *vao;
            VertexBuffer *vbo;

            std::vector<f32> vertices;
    };
};  // namespace bls
