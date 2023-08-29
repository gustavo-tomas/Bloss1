#include "renderer/model.hpp"
#include "renderer/texture.hpp"

#include <assimp/postprocess.h>

namespace bls
{
    Model::Model(const str& path, bool flip_uvs)
    {
        importer = new Assimp::Importer();
        this->path = path;
        this->bone_counter = 0;
        this->animator = nullptr;

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
        if (scene->HasAnimations())
        {
            // GlobalTransformationInverse
            scene->mRootNode->mTransformation.Inverse();
            for (u32 i = 0; i < scene->mNumAnimations; i++)
            {
                str name = scene->mAnimations[i]->mName.C_Str();
                animations[name] = std::make_unique<Animation>(scene->mRootNode, scene->mAnimations[i], bone_info_map, bone_counter);

                if (animator.get() == nullptr)
                    animator = std::make_unique<Animator>(animations[name].get());
            }
        }
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
        std::vector<Vertex> vertices;
        std::vector<u32> indices;
        std::vector<Texture*> textures;

        // Process vertex positions, normals and texture coordinates
        for (u32 i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            vec3 vector;

            // Set bone data
            set_vertex_bone_data_to_default(vertex);

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
        auto diffuse_maps = load_material_textures(material, aiTextureType_DIFFUSE);
        textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

        // Specular
        auto specular_maps = load_material_textures(material, aiTextureType_SPECULAR);
        textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

        // Normal
        auto normal_maps = load_material_textures(material, aiTextureType_NORMALS);
        textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());

        // Metalness
        auto metalness_maps = load_material_textures(material, aiTextureType_METALNESS);
        textures.insert(textures.end(), metalness_maps.begin(), metalness_maps.end());

        // Roughness (already loaded in metal ARM textures)
        // vector<Texture> roughnessMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness");
        // textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());

        // Ambient occlusion (AO)
        // vector<Texture> aoMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION, "texture_ao");
        // textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());

        // Bone weight
        extract_bone_weight_for_vertices(vertices, mesh);

        auto vao = VertexArray::create();
        vao->bind();

        auto vbo = VertexBuffer::create(static_cast<void*>(vertices.data()), vertices.size() * sizeof(Vertex));
        auto ebo = IndexBuffer::create(indices, indices.size());

        // Position
        vao->add_vertex_buffer(0, 3, ShaderDataType::Float, false, sizeof(Vertex), (void*) offsetof(Vertex, Vertex::position));

        // Normals
        vao->add_vertex_buffer(1, 3, ShaderDataType::Float, false, sizeof(Vertex), (void*) offsetof(Vertex, Vertex::normal));

        // Texture coords
        vao->add_vertex_buffer(2, 2, ShaderDataType::Float, false, sizeof(Vertex), (void*) offsetof(Vertex, Vertex::tex_coords));

        // Tangent
        vao->add_vertex_buffer(3, 3, ShaderDataType::Float, false, sizeof(Vertex), (void*) offsetof(Vertex, Vertex::tangent));

        // Bitangent
        vao->add_vertex_buffer(4, 3, ShaderDataType::Float, false, sizeof(Vertex), (void*) offsetof(Vertex, Vertex::bitangent));

        // Bone ids
        vao->add_vertex_buffer(5, 4, ShaderDataType::Int, false, sizeof(Vertex), (void*) offsetof(Vertex, Vertex::bone_ids));

        // Weights
        vao->add_vertex_buffer(6, 4, ShaderDataType::Float, false, sizeof(Vertex), (void*) offsetof(Vertex, Vertex::weights));

        return new Mesh(vao, vbo, ebo, vertices, indices, textures);
    }

    std::vector<Texture*> Model::load_material_textures(aiMaterial* mat, aiTextureType type)
    {
        str directory = path.substr(0, path.find_last_of('/'));

        std::vector<Texture*> textures;
        for (u32 i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            TextureType texture_type;
            switch (type)
            {
                case aiTextureType_DIFFUSE:   texture_type = TextureType::Diffuse;   break;
                case aiTextureType_SPECULAR:  texture_type = TextureType::Specular;  break;
                case aiTextureType_NORMALS:   texture_type = TextureType::Normal;    break;
                case aiTextureType_METALNESS: texture_type = TextureType::Metalness; break;

                default:
                    std::cerr << "invalid texture type: '" << type << "'\n";
                    break;
            }

            // @TODO: might wanna check if texture was already loaded for another mesh
            auto texture = Texture::create(str.C_Str(), directory + "/" + str.C_Str(), texture_type);
            textures.push_back(texture.get());
        }

        return textures;
    }

    auto& Model::get_bone_info_map()
    {
        return bone_info_map;
    };

    i32& Model::get_bone_count()
    {
        return bone_counter;
    };

    void Model::set_vertex_bone_data_to_default(Vertex& vertex)
    {
        for (i32 i = 0; i < MAX_BONE_PER_VERTEX; i++)
        {
            vertex.bone_ids[i] = -1;
            vertex.weights[i] = 0.0f;
        }
    }

    void Model::set_vertex_bone_data(Vertex& vertex, i32 bone_id, f32 weight)
    {
        for (i32 i = 0; i < MAX_BONE_PER_VERTEX; i++)
        {
            if (vertex.bone_ids[i] < 0)
            {
                vertex.weights[i] = weight;
                vertex.bone_ids[i] = bone_id;
                break;
            }
        }
    }

    void Model::extract_bone_weight_for_vertices(std::vector<Vertex>& vertices, aiMesh* mesh)
    {
        for (u32 boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
        {
            i32 boneID = -1;
            str bone_name = mesh->mBones[boneIndex]->mName.C_Str();

            if (bone_info_map.find(bone_name) == bone_info_map.end())
            {
                BoneInfo newBoneInfo;
                newBoneInfo.id = bone_counter;
                newBoneInfo.offset = ass_mat_to_glm_mat(mesh->mBones[boneIndex]->mOffsetMatrix);
                bone_info_map[bone_name] = newBoneInfo;
                boneID = bone_counter;
                bone_counter++;
            }

            else
                boneID = bone_info_map[bone_name].id;

            assert(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            i32 numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (i32 weightIndex = 0; weightIndex < numWeights; weightIndex++)
            {
                i32 vertexId = weights[weightIndex].mVertexId;
                f32 weight = weights[weightIndex].mWeight;
                assert(vertexId <= static_cast<i32>(vertices.size()));
                set_vertex_bone_data(vertices[vertexId], boneID, weight);
            }
        }
    }

    // Bone
    // -----------------------------------------------------------------------------------------------------------------
    Bone::Bone(const str& name, i32 id, const aiNodeAnim* channel)
    {
        this->name = name;
        this->id = id;
        this->local_transform = mat4(1.0f);

        num_positions = channel->mNumPositionKeys;
        num_rotations = channel->mNumRotationKeys;
        num_scalings = channel->mNumScalingKeys;

        for (i32 positionIndex = 0; positionIndex < num_positions; positionIndex++)
        {
            aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
            f32 timeStamp = channel->mPositionKeys[positionIndex].mTime;
            KeyPosition data = { };
            data.position = ass_vec_to_glm_vec(aiPosition);
            data.time_stamp = timeStamp;
            positions.push_back(data);
        }

        for (i32 rotationIndex = 0; rotationIndex < num_rotations; rotationIndex++)
        {
            aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
            f32 timeStamp = channel->mRotationKeys[rotationIndex].mTime;
            KeyRotation data = { };
            data.orientation = ass_quat_to_glm_quat(aiOrientation);
            data.time_stamp = timeStamp;
            rotations.push_back(data);
        }

        for (i32 keyIndex = 0; keyIndex < num_scalings; keyIndex++)
        {
            aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
            f32 timeStamp = channel->mScalingKeys[keyIndex].mTime;
            KeyScale data = { };
            data.scale = ass_vec_to_glm_vec(scale);
            data.time_stamp = timeStamp;
            scales.push_back(data);
        }
    }

    Bone::~Bone()
    {
        std::cout << "bone destroyed successfully\n";
    }

    void Bone::update(f32 animation_time)
    {
        mat4 translation = interpolate_position(animation_time);
        mat4 rotation = interpolate_rotation(animation_time);
        mat4 scale = interpolate_scaling(animation_time);
        local_transform = translation * rotation * scale;
    }

    i32 Bone::get_position_index(f32 animation_time)
    {
        for (i32 index = 0; index < num_positions - 1; index++)
        {
            if (animation_time < positions[index + 1].time_stamp)
            {
                return index;
            }
        }

        assert(false);
        // return -1; // @TODO: handle errors
    }

    i32 Bone::get_rotation_index(f32 animation_time)
    {
        for (i32 index = 0; index < num_rotations - 1; index++)
            if (animation_time < rotations[index + 1].time_stamp)
                return index;

        assert(false);
    }

    i32 Bone::get_scale_index(f32 animation_time)
    {
        for (i32 index = 0; index < num_scalings - 1; index++)
            if (animation_time < scales[index + 1].time_stamp)
                return index;

        assert(false);
    }

    f32 Bone::get_scale_factor(f32 last_time_stamp, f32 next_time_stamp, f32 animation_time)
    {
        f32 scale_factor = 0.0f;
        f32 midWayLength = animation_time - last_time_stamp;
        f32 framesDiff = next_time_stamp - last_time_stamp;
        scale_factor = midWayLength / framesDiff;

        return scale_factor;
    }

    mat4 Bone::interpolate_position(f32 animation_time)
    {
        if (1 == num_positions)
            return translate(mat4(1.0f), positions[0].position);

        i32 p0_index = get_position_index(animation_time);
        i32 p1_index = p0_index + 1;
        f32 scale_factor = get_scale_factor(positions[p0_index].time_stamp, positions[p1_index].time_stamp, animation_time);
        vec3 finalPosition = mix(positions[p0_index].position, positions[p1_index].position, scale_factor);

        return translate(mat4(1.0f), finalPosition);
    }

    mat4 Bone::interpolate_rotation(f32 animation_time)
    {
        if (1 == num_rotations)
        {
            auto rotation = normalize(rotations[0].orientation);
            return toMat4(rotation);
        }

        i32 p0_index = get_rotation_index(animation_time);
        i32 p1_index = p0_index + 1;
        f32 scale_factor = get_scale_factor(rotations[p0_index].time_stamp, rotations[p1_index].time_stamp, animation_time);
        quat final_rotation = slerp(rotations[p0_index].orientation, rotations[p1_index].orientation, scale_factor);
        final_rotation = normalize(final_rotation);

        return toMat4(final_rotation);
    }

    mat4 Bone::interpolate_scaling(f32 animation_time)
    {
        if (1 == num_scalings)
            return scale(mat4(1.0f), scales[0].scale);

        i32 p0_index = get_scale_index(animation_time);
        i32 p1_index = p0_index + 1;
        f32 scale_factor = get_scale_factor(scales[p0_index].time_stamp, scales[p1_index].time_stamp, animation_time);
        vec3 finalScale = mix(scales[p0_index].scale, scales[p1_index].scale, scale_factor);

        return scale(mat4(1.0f), finalScale);
    }

    mat4 Bone::get_local_transform()
    {
        return local_transform;
    }

    str Bone::get_bone_name()
    {
        return name;
    }

    i32 Bone::get_bone_id()
    {
        return id;
    }

    // Animation
    // -----------------------------------------------------------------------------------------------------------------
    Animation::Animation(const aiNode* root, const aiAnimation* animation, std::map<str, BoneInfo>& model_bone_info_map, i32& model_bone_count)
    {
        duration = animation->mDuration;
        ticks_per_second = animation->mTicksPerSecond;

        read_hierarchy_data(root_node, root);
        read_missing_bones(animation, model_bone_info_map, model_bone_count);
    }

    Animation::~Animation()
    {
        for (auto bone : bones) { delete bone; }
        std::cout << "animation destroyed successfully\n";
    }

    Bone* Animation::find_bone(const str& name)
    {
        auto it = find_if(bones.begin(), bones.end(), [&](Bone * bone)
        {
            return bone->get_bone_name() == name;
        });

        if (it == bones.end()) { return nullptr; }

        return *it;
    }

    void Animation::read_missing_bones(const aiAnimation* animation, std::map<str, BoneInfo>& model_bone_info_map, i32& model_bone_count)
    {
        // Reading channels (bones engaged in an animation and their keyframes)
        for (u32 i = 0; i < animation->mNumChannels; i++)
        {
            auto channel = animation->mChannels[i];
            str bone_name = channel->mNodeName.data;

            if (model_bone_info_map.find(bone_name) == model_bone_info_map.end())
            {
                model_bone_info_map[bone_name].id = model_bone_count;
                model_bone_count++;
            }
            bones.push_back(new Bone(channel->mNodeName.data, model_bone_info_map[channel->mNodeName.data].id, channel));
        }

        bone_info_map = model_bone_info_map;
    }

    void Animation::read_hierarchy_data(AssNodeData& dest, const aiNode* root)
    {
        assert(root);

        dest.name = root->mName.data;
        dest.transformation = ass_mat_to_glm_mat(root->mTransformation);
        dest.children_count = root->mNumChildren;

        for (u32 i = 0; i < root->mNumChildren; i++)
        {
            AssNodeData new_data;
            read_hierarchy_data(new_data, root->mChildren[i]);
            dest.children.push_back(new_data);
        }
    }

    f32 Animation::get_ticks_per_second()
    {
        return ticks_per_second;
    }

    f32 Animation::get_duration()
    {
        return duration;
    }

    AssNodeData& Animation::get_root_node()
    {
        return root_node;
    }

    std::map<str, BoneInfo>& Animation::get_bone_id_map()
    {
        return bone_info_map;
    }

    // Animator
    // -----------------------------------------------------------------------------------------------------------------
    Animator::Animator(Animation* animation)
    {
        current_time = 0.0;
        current_animation = animation;

        final_bone_matrices.reserve(MAX_BONE_MATRICES);

        for (u32 i = 0; i < MAX_BONE_MATRICES; i++)
            final_bone_matrices.push_back(mat4(1.0f));
    }

    Animator::~Animator()
    {
        std::cout << "animator destroyed successfully\n";
    }

    void Animator::update(f32 deltaTime)
    {
        if (current_animation)
        {
            current_time += current_animation->get_ticks_per_second() * deltaTime;
            current_time = fmod(current_time, current_animation->get_duration());
            calculate_bone_transform(&current_animation->get_root_node(), mat4(1.0f));
        }
    }

    void Animator::play(Animation* animation)
    {
        current_animation = animation;
        current_time = 0.0f;
    }

    void Animator::calculate_bone_transform(const AssNodeData* node, mat4 parent_transform)
    {
        str node_name = node->name;
        mat4 node_transform = node->transformation;

        Bone* bone = current_animation->find_bone(node_name);

        if (bone)
        {
            bone->update(current_time);
            node_transform = bone->get_local_transform();
        }

        mat4 global_transformation = parent_transform * node_transform;

        auto bone_info_map = current_animation->get_bone_id_map();
        if (bone_info_map.find(node_name) != bone_info_map.end())
        {
            i32 index = bone_info_map[node_name].id;
            mat4 offset = bone_info_map[node_name].offset;
            final_bone_matrices[index] = global_transformation * offset;
        }

        for (i32 i = 0; i < node->children_count; i++)
            calculate_bone_transform(&node->children[i], global_transformation);
    }

    std::vector<mat4> Animator::get_final_bone_matrices()
    {
        return final_bone_matrices;
    }
};
