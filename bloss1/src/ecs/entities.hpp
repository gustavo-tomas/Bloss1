#pragma once

/**
 * @brief The entities of the ECS. Might wanna split in other files if it gets too big.
 */

#include "core/game.hpp"
#include "ecs/ecs.hpp"
#include "renderer/buffers.hpp"

// @TODO: testing
std::vector<f32> cube_vertices =
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

namespace bls
{
    u32 player(ECS& ecs, const Transform& transform)
    {
        u32 id = ecs.get_id();

        std::vector<Mesh::Vertex> vertices(8, Mesh::Vertex());

        // Setup Mesh
        vertices[0].position = { -1.0f, -1.0f, 1.0f };
        vertices[1].position = {  1.0f, -1.0f, 1.0f };
        vertices[2].position = {  1.0f,  1.0f, 1.0f };
        vertices[3].position = { -1.0f,  1.0f, 1.0f };

        vertices[0].tex_coords = { 0.0f, 0.0f };
        vertices[1].tex_coords = { 1.0f, 0.0f };
        vertices[2].tex_coords = { 1.0f, 1.0f };
        vertices[3].tex_coords = { 0.0f, 1.0f };

        vertices[4].position = { -1.0f, -1.0f, -1.0f };
        vertices[5].position = {  1.0f, -1.0f, -1.0f };
        vertices[6].position = {  1.0f,  1.0f, -1.0f };
        vertices[7].position = { -1.0f,  1.0f, -1.0f };

        vertices[4].tex_coords = { 1.0f, 0.0f };
        vertices[5].tex_coords = { 0.0f, 0.0f };
        vertices[6].tex_coords = { 0.0f, 1.0f };
        vertices[7].tex_coords = { 1.0f, 1.0f };

        auto vao = VertexArray::create();
        vao->bind();

        auto vbo = VertexBuffer::create(static_cast<void*>(vertices.data()), vertices.size() * sizeof(Mesh::Vertex));
        auto ebo = IndexBuffer::create(indices, indices.size());

        // Position
        vao->add_vertex_buffer(0, 3, ShaderDataType::Float, false, sizeof(Mesh::Vertex), (void*) 0);

        // Normals
        vao->add_vertex_buffer(1, 3, ShaderDataType::Float, false, sizeof(Mesh::Vertex), (void*) offsetof(Mesh::Vertex, Mesh::Vertex::normal));

        // Texture coords
        vao->add_vertex_buffer(2, 2, ShaderDataType::Float, false, sizeof(Mesh::Vertex), (void*) offsetof(Mesh::Vertex, Mesh::Vertex::tex_coords));

        // Tangent
        vao->add_vertex_buffer(3, 3, ShaderDataType::Float, false, sizeof(Mesh::Vertex), (void*) offsetof(Mesh::Vertex, Mesh::Vertex::tangent));

        // Vertex bitangent
        vao->add_vertex_buffer(4, 3, ShaderDataType::Float, false, sizeof(Mesh::Vertex), (void*) offsetof(Mesh::Vertex, Mesh::Vertex::bitangent));

        // // Bone IDs
        // glEnableVertexAttribArray(5);
        // glVertexAttribIPointer(5, 4, GL_INT, sizeof(Mesh::Vertex), (void*) offsetof(Mesh::Vertex, Mesh::Vertex::boneIDs));

        // // Weights
        // glEnableVertexAttribArray(6);
        // glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void*) offsetof(Mesh::Vertex, Mesh::Vertex::weights));

        ecs.models[id] = std::make_unique<Mesh>(vao, vbo, ebo, vertices, indices);
        ecs.transforms[id] = std::make_unique<Transform>(transform);

        return id;
    }
};
