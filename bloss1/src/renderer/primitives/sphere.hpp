#pragma once

/**
 * @brief Sphere primitive with configurable number of segments and radius.
 */

#include "renderer/buffers.hpp"
#include "renderer/renderer.hpp"

namespace bls
{

    class Sphere
    {
        public:
            Sphere(Renderer &renderer,
                   const vec3 &position = vec3(0.0f),
                   f32 radius = 1.0f,
                   u32 x_segments = 32,
                   u32 y_segments = 32)
                : renderer(renderer)
            {
                // Setup vertices
                for (u32 x = 0; x <= x_segments; x++)
                {
                    for (u32 y = 0; y <= y_segments; y++)
                    {
                        f32 x_segment = static_cast<f32>(x) / static_cast<f32>(x_segments);
                        f32 y_segment = static_cast<f32>(y) / static_cast<f32>(y_segments);

                        f32 x_pos = cos(x_segment * 2.0f * M_PI) * sin(y_segment * M_PI) * radius;
                        f32 y_pos = cos(y_segment * M_PI) * radius;
                        f32 z_pos = sin(x_segment * 2.0f * M_PI) * sin(y_segment * M_PI) * radius;

                        positions.push_back(vec3(x_pos, y_pos, z_pos) + position);
                        uv.push_back(vec2(x_segment, y_segment));
                        normals.push_back(vec3(x_pos, y_pos, z_pos));
                    }
                }

                bool odd_row = false;
                for (u32 y = 0; y < y_segments; y++)
                {
                    if (!odd_row)
                    {
                        for (u32 x = 0; x <= x_segments; x++)
                        {
                            indices.push_back(y * (x_segments + 1) + x);
                            indices.push_back((y + 1) * (x_segments + 1) + x);
                        }
                    }

                    else
                    {
                        for (i32 x = x_segments; x >= 0; x--)
                        {
                            indices.push_back((y + 1) * (x_segments + 1) + x);
                            indices.push_back(y * (x_segments + 1) + x);
                        }
                    }
                    odd_row = !odd_row;
                }

                for (u32 i = 0; i < positions.size(); i++)
                {
                    vertices.push_back(positions[i].x);
                    vertices.push_back(positions[i].y);
                    vertices.push_back(positions[i].z);

                    if (normals.size() > 0)
                    {
                        vertices.push_back(normals[i].x);
                        vertices.push_back(normals[i].y);
                        vertices.push_back(normals[i].z);
                    }

                    if (uv.size() > 0)
                    {
                        vertices.push_back(uv[i].x);
                        vertices.push_back(uv[i].y);
                    }
                }

                // Setup VAO
                vao = VertexArray::create();
                vao->bind();

                vbo = VertexBuffer::create(static_cast<void *>(vertices.data()), vertices.size() * sizeof(f32));
                ebo = IndexBuffer::create(indices, indices.size());

                // Position
                vao->add_vertex_buffer(0, 3, ShaderDataType::Float, false, 8 * sizeof(f32), (void *)0);

                // Normal
                vao->add_vertex_buffer(1, 3, ShaderDataType::Float, false, 8 * sizeof(f32), (void *)(3 * sizeof(f32)));

                // Texture coordinates
                vao->add_vertex_buffer(2, 2, ShaderDataType::Float, false, 8 * sizeof(f32), (void *)(6 * sizeof(f32)));
            };

            ~Sphere()
            {
                delete vao;
                delete vbo;
                delete ebo;
            };

            void render()
            {
                vao->bind();
                renderer.draw_indexed(RenderingMode::TriangleStrip, indices.size());
                vao->unbind();
            };

        private:
            Renderer &renderer;
            VertexArray *vao;
            VertexBuffer *vbo;
            IndexBuffer *ebo;

            std::vector<f32> vertices;
            std::vector<u32> indices;

            std::vector<vec3> positions;
            std::vector<vec2> uv;
            std::vector<vec3> normals;
    };
};  // namespace bls
