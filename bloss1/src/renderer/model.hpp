#pragma once

/**
 * @brief
 *
 */

#include "renderer/buffers.hpp"
#include "renderer/texture.hpp"
#include "renderer/assimp_utils.hpp"
#include "math/math.hpp"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#define MAX_BONE_PER_VERTEX 4

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

                i32 bone_ids[MAX_BONE_PER_VERTEX];
                f32 weights[MAX_BONE_PER_VERTEX];
            };

            struct BoneInfo
            {
                i32 id;
                mat4 offset;
            };

            Mesh(VertexArray* vao, VertexBuffer* vbo, IndexBuffer* ebo,
                 const std::vector<Vertex>& vertices, const std::vector<u32>& indices, const std::vector<Texture*>& textures)
                : vao(vao), vbo(vbo), ebo(ebo), vertices(vertices), indices(indices), textures(textures)
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
            std::vector<Texture*> textures;
    };

    class Model
    {
        public:
            Model(const str& path, bool flip_uvs);
            ~Model();

            static std::shared_ptr<Model> create(const str& name, const str& path, bool flip_uvs); // i know i know

            str path;
            std::vector<Mesh*> meshes;
            std::map<str, Mesh::BoneInfo> bone_info_map;
            i32 bone_counter;

        private:

            // Helper methods
            void process_node(aiNode* node, const aiScene* scene);
            Mesh* process_mesh(aiMesh* mesh, const aiScene* scene);
            std::vector<Texture*> load_material_textures(aiMaterial* mat, aiTextureType type);

            auto& get_bone_info_map() { return bone_info_map; };
            i32& get_bone_count() { return bone_counter; };

            void SetVertexBoneDataToDefault(Mesh::Vertex& vertex)
            {
                for (i32 i = 0; i < MAX_BONE_PER_VERTEX; i++)
                {
                    vertex.bone_ids[i] = -1;
                    vertex.weights[i] = 0.0f;
                }
            }

            void SetVertexBoneData(Mesh::Vertex& vertex, i32 boneID, f32 weight)
            {
                for (i32 i = 0; i < MAX_BONE_PER_VERTEX; i++)
                {
                    if (vertex.bone_ids[i] < 0)
                    {
                        vertex.weights[i] = weight;
                        vertex.bone_ids[i] = boneID;
                        break;
                    }
                }
            }

            void ExtractBoneWeightForVertices(std::vector<Mesh::Vertex>& vertices, aiMesh* mesh)
            {
                for (u32 boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
                {
                    i32 boneID = -1;
                    str boneName = mesh->mBones[boneIndex]->mName.C_Str();

                    if (bone_info_map.find(boneName) == bone_info_map.end())
                    {
                        Mesh::BoneInfo newBoneInfo;
                        newBoneInfo.id = bone_counter;
                        newBoneInfo.offset = assMatToGlmMat(mesh->mBones[boneIndex]->mOffsetMatrix);
                        bone_info_map[boneName] = newBoneInfo;
                        boneID = bone_counter;
                        bone_counter++;
                    }

                    else
                        boneID = bone_info_map[boneName].id;

                    assert(boneID != -1);
                    auto weights = mesh->mBones[boneIndex]->mWeights;
                    i32 numWeights = mesh->mBones[boneIndex]->mNumWeights;

                    for (i32 weightIndex = 0; weightIndex < numWeights; weightIndex++)
                    {
                        i32 vertexId = weights[weightIndex].mVertexId;
                        f32 weight = weights[weightIndex].mWeight;
                        assert(vertexId <= static_cast<i32>(vertices.size()));
                        SetVertexBoneData(vertices[vertexId], boneID, weight);
                    }
                }
            }

            Assimp::Importer* importer; // @TODO: find a better solution
    };
};
