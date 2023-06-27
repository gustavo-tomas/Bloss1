#pragma once

/**
 * @brief The components of the ECS. Might wanna split in other files if it gets too big.
 */

#include "ecs/ecs.hpp"
#include "math/math.hpp"
#include "renderer/buffers.hpp"

namespace bls
{
    // Component: contain the data
    class Component
    {
        public:
            virtual ~Component() { }
    };

    class Transform : public Component
    {
        public:
            Transform(const vec3& position = vec3(0.0f), const vec3& rotation = vec3(0.0f), const vec3& scale = vec3(1.0f))
                : position(position), rotation(rotation), scale(scale) { }

            vec3 position;
            vec3 rotation;
            vec3 scale;
    };

    class Mesh : public Component
    {
        public:
            struct Vertex
            {
                vec3 position;
                vec3 normal;
                vec2 tex_coords;
                vec3 tangent;
                vec3 bitangent;

                // int boneIDs[MAX_BONE_PER_VERTEX];
                // float weights[MAX_BONE_PER_VERTEX];
            };

            Mesh(VertexArray* vao, VertexBuffer* vbo, IndexBuffer* ebo,
                 const std::vector<Vertex>& vertices, const std::vector<u32>& indices)
                : vao(vao), vbo(vbo), ebo(ebo), vertices(vertices), indices(indices)
            { }

            ~Mesh()
            {
                delete vao;
                delete vbo;
                delete ebo;
            }

            VertexArray* vao;
            VertexBuffer* vbo;
            IndexBuffer* ebo;

            std::vector<Vertex> vertices;
            std::vector<u32> indices;
    };
};
