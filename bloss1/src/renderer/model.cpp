#include "renderer/model.hpp"
#include "renderer/texture.hpp"

#include <assimp/postprocess.h>

namespace bls
{
    Model::Model(const str& path, bool flip_uvs)
    {
        importer = new Assimp::Importer();
        this->path = path;

        u32 flags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace;
        if (flip_uvs)
            flags |= aiProcess_FlipUVs;

        const aiScene* scene = importer->ReadFile(path, flags);

        std::cout << "loading model: '" << path << "'\n";

        if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
        {
            std::cerr << "error loading model: '" << importer->GetErrorString() << "'\n";
            return;
        }

        // Process nodes
        process_node(scene->mRootNode, scene);

        // Process animations
        // if (scene->HasAnimations())
        // {
        //     // GlobalTransformationInverse
        //     scene->mRootNode->mTransformation.Inverse();
        //     for (u32 i = 0; i < scene->mNumAnimations; i++)
        //     {
        //         str name = scene->mAnimations[i]->mName.C_Str();
        //         modelData->animations[name] = new Animation(scene->mRootNode, scene->mAnimations[i], modelData->boneInfoMap, modelData->boneCounter);
        //     }
        // }
    }

    Model::~Model()
    {
        delete importer;
        for (auto& mesh : meshes)
            delete mesh;
    }

    void Model::process_node(aiNode* node, const aiScene* scene)
    {
        // Process all the node's meshes (if any)
        for (u32 i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(process_mesh(mesh, scene));
        }

        // Then do the same for each of its children
        for (u32 i = 0; i < node->mNumChildren; i++)
            process_node(node->mChildren[i], scene);
    }

    Mesh* Model::process_mesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Mesh::Vertex> vertices;
        std::vector<u32> indices;
        std::vector<Texture> textures;

        // Process vertex positions, normals and texture coordinates
        for (u32 i = 0; i < mesh->mNumVertices; i++)
        {
            Mesh::Vertex vertex;
            vec3 vector;

            // Set bone data
            // for (u32 j = 0; j < MAX_BONE_PER_VERTEX; j++)
            // {
            //     vertex.boneIDs[j] = -1;
            //     vertex.weights[j] = 0.0f;
            // }

            // Position
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.position = vector;

            // Normal
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;

            // Tangents
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.tangent = vector;

            // Bitangents
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.bitangent = vector;

            // Texture coordinates
            if (mesh->mTextureCoords[0])
                vertex.tex_coords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

            // No texture present
            else
                vertex.tex_coords = vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        // Process indices
        for (u32 i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (u32 j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // Process material
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // Diffuse
        // std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        // textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        // // Specular
        // std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        // textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        // // Normal
        // std::vector<Texture> normalMaps = LoadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
        // textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        // // Metalness
        // std::vector<Texture> metalnessMaps = LoadMaterialTextures(material, aiTextureType_METALNESS, "texture_metalness");
        // textures.insert(textures.end(), metalnessMaps.begin(), metalnessMaps.end());

        // Roughness (already loaded in metal ARM textures)
        // vector<Texture> roughnessMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness");
        // textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());

        // Ambient occlusion (AO)
        // vector<Texture> aoMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION, "texture_ao");
        // textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());

        // Bone weight
        // ExtractBoneWeightForVertices(vertices, mesh);

        // return new Mesh(vertices, indices, textures);

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

        // Bitangent
        vao->add_vertex_buffer(4, 3, ShaderDataType::Float, false, sizeof(Mesh::Vertex), (void*) offsetof(Mesh::Vertex, Mesh::Vertex::bitangent));

        return new Mesh(vao, vbo, ebo, vertices, indices);
    }
};
