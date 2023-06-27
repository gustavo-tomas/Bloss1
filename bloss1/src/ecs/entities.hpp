#pragma once

/**
 * @brief The entities of the ECS. Might wanna split in other files if it gets too big.
 */

#include "core/game.hpp"
#include "ecs/ecs.hpp"
#include "renderer/buffers.hpp"

// @TODO: testing
f32 cube_vertices[48] =
{
    // Front face       // Texture coords   // Vertice
    -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,         // (0)
    1.0f,  -1.0f, 1.0f, 1.0f, 0.0f,         // (1)
    1.0f,   1.0f, 1.0f, 1.0f, 1.0f,         // (2)
    -1.0f,  1.0f, 1.0f, 0.0f, 1.0f,         // (3)

    // Back face         // Texture coords  // Vertice
    -1.0f, -1.0f, -1.0f, 1.0f, 0.0f,        // (4)
    1.0f,  -1.0f, -1.0f, 0.0f, 0.0f,        // (5)
    1.0f,   1.0f, -1.0f, 0.0f, 1.0f,        // (6)
    -1.0f,  1.0f, -1.0f, 1.0f, 1.0f,        // (7)
};

u32 cube_indices[36] =
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

namespace bls
{
    u32 player(ECS& ecs, const Transform& transform)
    {
        u32 id = ecs.get_id();

        // Setup Mesh
        auto vao = VertexArray::create();
        vao->bind();

        auto vbo = VertexBuffer::create(cube_vertices, sizeof(cube_vertices));
        auto ebo = IndexBuffer::create(cube_indices, sizeof(cube_indices) / sizeof(u32));

        // Position
        vao->add_vertex_buffer(0, 3, ShaderDataType::Float, false, 5 * sizeof(f32), (void*) 0);

        // Texture
        vao->add_vertex_buffer(1, 2, ShaderDataType::Float, false, 5 * sizeof(f32), (void*) (3 * sizeof(f32)));

        std::vector<f32> vertices(cube_vertices, cube_vertices + 48);
        std::vector<u32> indices(cube_indices, cube_indices + 36);

        ecs.models[id] = std::make_unique<Mesh>(vao, vbo, ebo, vertices, indices);
        ecs.transforms[id] = std::make_unique<Transform>(transform);

        return id;
    }
};
