#pragma once

/**
 * @brief Simple box primitive with normals and texture coords.
 */

#include "renderer/buffers.hpp"
#include "renderer/renderer.hpp"

namespace bls
{
    class Box
    {
        public:
            Box(Renderer &renderer, const vec3 &position = vec3(0.0f), const vec3 &dimensions = vec3(1.0f))
                : renderer(renderer)
            {
                vertices = {
                    // Front face
                    // Position // Normals // Texture coordinates
                    position.x + -dimensions.x,
                    position.y + -dimensions.y,
                    position.z + dimensions.z,
                    0.0f,
                    0.0f,
                    1.0f,
                    0.0f,
                    0.0f,  // (0)
                    position.x + dimensions.x,
                    position.y + -dimensions.y,
                    position.z + dimensions.z,
                    0.0f,
                    0.0f,
                    1.0f,
                    1.0f,
                    0.0f,  // (1)
                    position.x + dimensions.x,
                    position.y + dimensions.y,
                    position.z + dimensions.z,
                    0.0f,
                    0.0f,
                    1.0f,
                    1.0f,
                    1.0f,  // (2)
                    position.x + -dimensions.x,
                    position.y + dimensions.y,
                    position.z + dimensions.z,
                    0.0f,
                    0.0f,
                    1.0f,
                    0.0f,
                    1.0f,  // (3)

                    // Back face
                    // Position // Normals // Texture coordinates
                    position.x + -dimensions.x,
                    position.y + -dimensions.y,
                    position.z + -dimensions.z,
                    0.0f,
                    0.0f,
                    -1.0f,
                    0.0f,
                    0.0f,  // (4)
                    position.x + dimensions.x,
                    position.y + -dimensions.y,
                    position.z + -dimensions.z,
                    0.0f,
                    0.0f,
                    -1.0f,
                    1.0f,
                    0.0f,  // (5)
                    position.x + dimensions.x,
                    position.y + dimensions.y,
                    position.z + -dimensions.z,
                    0.0f,
                    0.0f,
                    -1.0f,
                    1.0f,
                    1.0f,  // (6)
                    position.x + -dimensions.x,
                    position.y + dimensions.y,
                    position.z + -dimensions.z,
                    0.0f,
                    0.0f,
                    -1.0f,
                    0.0f,
                    1.0f  // (7)
                };

                // Setup cube VAO
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

            ~Box()
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

            std::vector<f32> vertices;

            std::vector<u32> indices = {
                // Front face
                0,
                1,
                2,
                2,
                3,
                0,

                // Right face
                1,
                5,
                6,
                6,
                2,
                1,

                // Back face
                7,
                6,
                5,
                5,
                4,
                7,

                // Left face
                4,
                0,
                3,
                3,
                7,
                4,

                // Bottom face
                4,
                5,
                1,
                1,
                0,
                4,

                // Top face
                3,
                2,
                6,
                6,
                7,
                3};
    };
};  // namespace bls
