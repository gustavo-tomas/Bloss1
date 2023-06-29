#pragma once

/**
 * @brief
 *
 */

#include "renderer/buffers.hpp"
#include "math/math.hpp"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>

// @TODO: for now model = meshes + textures
namespace bls
{
    class Mesh
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

    class Model
    {
        public:
            Model(const str& path, bool flip_uvs);
            ~Model();

            static std::shared_ptr<Model> create(const str& name, const str& path, bool flip_uvs); // i know i know

            str path;
            std::vector<Mesh*> meshes;

        private:

            // Helper methods
            void process_node(aiNode* node, const aiScene* scene);
            Mesh* process_mesh(aiMesh* mesh, const aiScene* scene);

            Assimp::Importer* importer; // @TODO: find a better solution
    };
};
