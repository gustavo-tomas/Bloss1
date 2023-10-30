#include "renderer/model.hpp"

#include "assimp/postprocess.h"
#include "core/logger.hpp"
#include "renderer/texture.hpp"

namespace bls
{
    Model::Model(const str &path, bool flip_uvs)
    {
        importer = new Assimp::Importer();
        this->path = path;
        this->flip_uvs = flip_uvs;
        this->bone_counter = 0;
        this->animator = nullptr;

        u32 flags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace;
        if (flip_uvs) flags |= aiProcess_FlipUVs;

        const aiScene *scene = importer->ReadFile(path, flags);

        LOG_INFO("loading model '%s'", path.c_str());

        if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
        {
            LOG_ERROR("error loading model '%s'", importer->GetErrorString());
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
                animations[name] = std::make_unique<SkeletalAnimation>(
                    scene->mRootNode, scene->mAnimations[i], bone_info_map, bone_counter);

                if (animator.get() == nullptr) animator = std::make_unique<Animator>(animations[name].get());
            }

            LOG_INFO("animations found:");
            for (const auto &[name, anim] : animations) LOG_INFO("> %s", name.c_str());
        }
    }

    Model::~Model()
    {
        delete importer;
        for (const auto &mesh : meshes) delete mesh;
    }

    void Model::process_node(aiNode *node, const aiScene *scene)
    {
        // Process all the node's meshes (if any)
        for (u32 i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(process_mesh(mesh, scene));
        }

        // Then do the same for each of its children
        for (u32 i = 0; i < node->mNumChildren; i++) process_node(node->mChildren[i], scene);
    }

    Mesh *Model::process_mesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<Vertex> vertices;
        std::vector<u32> indices;
        std::vector<std::shared_ptr<Texture>> textures;

        // Process vertex positions, normals and texture coordinates
        for (u32 i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex = {};
            vec3 vector;

            // Set bone data
            set_vertex_bone_data_to_default(vertex);

            // Position
            if (mesh->HasPositions())
            {
                vector.x = mesh->mVertices[i].x;
                vector.y = mesh->mVertices[i].y;
                vector.z = mesh->mVertices[i].z;
                vertex.position = vector;
            }

            // Normal
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.normal = vector;
            }

            // Tangents
            if (mesh->HasTangentsAndBitangents())
            {
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.tangent = vector;

                // Bitangents
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.bitangent = vector;
            }

            // Texture coordinates
            if (mesh->HasTextureCoords(0))
                vertex.tex_coords = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};

            vertices.push_back(vertex);
        }

        // Process indices
        for (u32 i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (u32 j = 0; j < face.mNumIndices; j++) indices.push_back(face.mIndices[j]);
        }

        // Process material
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        // Load textures (or default textures if none is found)
        typedef struct TextureParams
        {
                aiTextureType ai_texture_type;
                TextureType texture_type;
        } TextureParams;

        // Roughness and AO should be loaded with metalness -> ARM textures
        std::vector<TextureParams> texture_params = {{aiTextureType_DIFFUSE, TextureType::Diffuse},
                                                     {aiTextureType_SPECULAR, TextureType::Specular},
                                                     {aiTextureType_NORMALS, TextureType::Normal},
                                                     {aiTextureType_METALNESS, TextureType::Metalness},
                                                     {aiTextureType_EMISSIVE, TextureType::Emissive}};

        for (const auto &[ai_texture_type, texture_type] : texture_params)
        {
            auto texture_maps = load_material_textures(material, ai_texture_type);
            if (texture_maps.empty())
                textures.push_back(Texture::get_default(texture_type));
            else
                textures.insert(textures.end(), texture_maps.begin(), texture_maps.end());
        }

        // Bone weight
        extract_bone_weight_for_vertices(vertices, mesh);

        auto vao = VertexArray::create();
        vao->bind();

        auto vbo = VertexBuffer::create(static_cast<void *>(vertices.data()), vertices.size() * sizeof(Vertex));
        auto ebo = IndexBuffer::create(indices, indices.size());

        // Position
        vao->add_vertex_buffer(0,
                               3,
                               ShaderDataType::Float,
                               false,
                               sizeof(Vertex),
                               reinterpret_cast<void *>(offsetof(Vertex, Vertex::position)));

        // Normals
        vao->add_vertex_buffer(1,
                               3,
                               ShaderDataType::Float,
                               false,
                               sizeof(Vertex),
                               reinterpret_cast<void *>(offsetof(Vertex, Vertex::normal)));

        // Texture coords
        vao->add_vertex_buffer(2,
                               2,
                               ShaderDataType::Float,
                               false,
                               sizeof(Vertex),
                               reinterpret_cast<void *>(offsetof(Vertex, Vertex::tex_coords)));

        // Tangent
        vao->add_vertex_buffer(3,
                               3,
                               ShaderDataType::Float,
                               false,
                               sizeof(Vertex),
                               reinterpret_cast<void *>(offsetof(Vertex, Vertex::tangent)));

        // Bitangent
        vao->add_vertex_buffer(4,
                               3,
                               ShaderDataType::Float,
                               false,
                               sizeof(Vertex),
                               reinterpret_cast<void *>(offsetof(Vertex, Vertex::bitangent)));

        // Bone ids
        vao->add_vertex_buffer(5,
                               4,
                               ShaderDataType::Int,
                               false,
                               sizeof(Vertex),
                               reinterpret_cast<void *>(offsetof(Vertex, Vertex::bone_ids)));

        // Weights
        vao->add_vertex_buffer(6,
                               4,
                               ShaderDataType::Float,
                               false,
                               sizeof(Vertex),
                               reinterpret_cast<void *>(offsetof(Vertex, Vertex::weights)));

        return new Mesh(vao, vbo, ebo, vertices, indices, textures);
    }

    std::vector<std::shared_ptr<Texture>> Model::load_material_textures(aiMaterial *mat, aiTextureType type)
    {
        str directory = path.substr(0, path.find_last_of('/'));

        std::vector<std::shared_ptr<Texture>> textures;
        for (u32 i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            TextureType texture_type;
            switch (type)
            {
                case aiTextureType_DIFFUSE:
                    texture_type = TextureType::Diffuse;
                    break;
                case aiTextureType_SPECULAR:
                    texture_type = TextureType::Specular;
                    break;
                case aiTextureType_NORMALS:
                    texture_type = TextureType::Normal;
                    break;
                case aiTextureType_METALNESS:
                    texture_type = TextureType::Metalness;
                    break;
                case aiTextureType_EMISSIVE:
                    texture_type = TextureType::Emissive;
                    break;

                default:
                    LOG_ERROR(
                        "invalid texture type for "
                        "material '%d'",
                        type);
                    break;
            }

            auto texture = Texture::create(str.C_Str(), directory + "/" + str.C_Str(), texture_type);
            textures.push_back(texture);
        }

        return textures;
    }

    auto &Model::get_bone_info_map()
    {
        return bone_info_map;
    };

    i32 &Model::get_bone_count()
    {
        return bone_counter;
    };

    void Model::set_vertex_bone_data_to_default(Vertex &vertex)
    {
        for (i32 i = 0; i < MAX_BONE_PER_VERTEX; i++)
        {
            vertex.bone_ids[i] = -1;
            vertex.weights[i] = 0.0f;
        }
    }

    void Model::set_vertex_bone_data(Vertex &vertex, i32 bone_id, f32 weight)
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

    void Model::extract_bone_weight_for_vertices(std::vector<Vertex> &vertices, aiMesh *mesh)
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
            u32 numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (u32 weightIndex = 0; weightIndex < numWeights; weightIndex++)
            {
                u32 vertexId = weights[weightIndex].mVertexId;
                f32 weight = weights[weightIndex].mWeight;
                assert(vertexId <= static_cast<u32>(vertices.size()));
                set_vertex_bone_data(vertices[vertexId], boneID, weight);
            }
        }
    }

    // Bone
    // -----------------------------------------------------------------------------------------------------------------
    Bone::Bone(const str &name, i32 id, const aiNodeAnim *channel)
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
            KeyPosition data = {};
            data.position = ass_vec_to_glm_vec(aiPosition);
            data.time_stamp = timeStamp;
            positions.push_back(data);
        }

        for (i32 rotationIndex = 0; rotationIndex < num_rotations; rotationIndex++)
        {
            aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
            f32 timeStamp = channel->mRotationKeys[rotationIndex].mTime;
            KeyRotation data = {};
            data.orientation = ass_quat_to_glm_quat(aiOrientation);
            data.time_stamp = timeStamp;
            rotations.push_back(data);
        }

        for (i32 keyIndex = 0; keyIndex < num_scalings; keyIndex++)
        {
            aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
            f32 timeStamp = channel->mScalingKeys[keyIndex].mTime;
            KeyScale data = {};
            data.scale = ass_vec_to_glm_vec(scale);
            data.time_stamp = timeStamp;
            scales.push_back(data);
        }
    }

    Bone::~Bone()
    {
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
            if (animation_time < positions[index + 1].time_stamp) return index;

        assert(false);
    }

    i32 Bone::get_rotation_index(f32 animation_time)
    {
        for (i32 index = 0; index < num_rotations - 1; index++)
            if (animation_time < rotations[index + 1].time_stamp) return index;

        assert(false);
    }

    i32 Bone::get_scale_index(f32 animation_time)
    {
        for (i32 index = 0; index < num_scalings - 1; index++)
            if (animation_time < scales[index + 1].time_stamp) return index;

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
        if (1 == num_positions) return translate(mat4(1.0f), positions[0].position);

        i32 p0_index = get_position_index(animation_time);
        i32 p1_index = p0_index + 1;
        f32 scale_factor =
            get_scale_factor(positions[p0_index].time_stamp, positions[p1_index].time_stamp, animation_time);
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
        f32 scale_factor =
            get_scale_factor(rotations[p0_index].time_stamp, rotations[p1_index].time_stamp, animation_time);
        quat final_rotation = slerp(rotations[p0_index].orientation, rotations[p1_index].orientation, scale_factor);
        final_rotation = normalize(final_rotation);

        return toMat4(final_rotation);
    }

    mat4 Bone::interpolate_scaling(f32 animation_time)
    {
        if (1 == num_scalings) return scale(mat4(1.0f), scales[0].scale);

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

    // Skeletal animation
    // -----------------------------------------------------------------------------------------------------------------
    SkeletalAnimation::SkeletalAnimation(const aiNode *root,
                                         const aiAnimation *animation,
                                         std::map<str, BoneInfo> &model_bone_info_map,
                                         i32 &model_bone_count)
    {
        duration = animation->mDuration;
        ticks_per_second = animation->mTicksPerSecond;
        name = animation->mName.C_Str();

        read_hierarchy_data(root_node, root);
        read_missing_bones(animation, model_bone_info_map, model_bone_count);
    }

    SkeletalAnimation::~SkeletalAnimation()
    {
        for (auto bone : bones)
        {
            delete bone;
        }
    }

    Bone *SkeletalAnimation::find_bone(const str &name)
    {
        auto it = find_if(bones.begin(), bones.end(), [&](Bone *bone) { return bone->get_bone_name() == name; });

        if (it == bones.end())
        {
            return nullptr;
        }

        return *it;
    }

    void SkeletalAnimation::read_missing_bones(const aiAnimation *animation,
                                               std::map<str, BoneInfo> &model_bone_info_map,
                                               i32 &model_bone_count)
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
            bones.push_back(
                new Bone(channel->mNodeName.data, model_bone_info_map[channel->mNodeName.data].id, channel));
        }

        bone_info_map = model_bone_info_map;
    }

    void SkeletalAnimation::read_hierarchy_data(AssNodeData &dest, const aiNode *root)
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

    str SkeletalAnimation::get_name()
    {
        return name;
    }

    f32 SkeletalAnimation::get_ticks_per_second()
    {
        return ticks_per_second;
    }

    f32 SkeletalAnimation::get_duration()
    {
        return duration;
    }

    f32 SkeletalAnimation::get_duration_seconds()
    {
        return duration / ticks_per_second;
    }

    AssNodeData &SkeletalAnimation::get_root_node()
    {
        return root_node;
    }

    std::map<str, BoneInfo> &SkeletalAnimation::get_bone_id_map()
    {
        return bone_info_map;
    }

    // Animator
    // -----------------------------------------------------------------------------------------------------------------
    Animator::Animator(SkeletalAnimation *animation)
    {
        current_time = 0.0f;
        previous_time = 0.0f;
        current_animation = animation;

        final_bone_matrices.reserve(MAX_BONE_MATRICES);

        for (u32 i = 0; i < MAX_BONE_MATRICES; i++) final_bone_matrices.push_back(mat4(1.0f));
    }

    Animator::~Animator()
    {
    }

    void Animator::update(f32 dt)
    {
        if (current_animation)
        {
            current_time += current_animation->get_ticks_per_second() * dt;
            current_time = fmod(current_time, current_animation->get_duration());
            calculate_bone_transform(&current_animation->get_root_node(), mat4(1.0f));
        }
    }

    void Animator::play(SkeletalAnimation *animation)
    {
        current_animation = animation;
    }

    void Animator::crossfade_from(SkeletalAnimation *prev_animation, f32 blend_factor, bool synchronize)
    {
        this->previous_animation = prev_animation;
        this->blend_factor = blend_factor;

        auto temp = current_time;

        if (synchronize)
        {
            const f32 ratio = current_animation->get_duration() / prev_animation->get_duration();
            current_time = previous_time * ratio;
        }

        else
            current_time = 0.0f;

        previous_time = temp;
    }

    void Animator::update_blended(f32 dt)
    {
        blend_factor = clamp(blend_factor + dt, 0.0f, 1.0f);

        current_time += current_animation->get_ticks_per_second() * dt;
        current_time = fmod(current_time, current_animation->get_duration());

        previous_time += previous_animation->get_ticks_per_second() * dt;
        previous_time = fmod(previous_time, previous_animation->get_duration());

        calculate_blended_bone_transform(previous_animation,
                                         &previous_animation->get_root_node(),
                                         current_animation,
                                         &current_animation->get_root_node(),
                                         previous_time,
                                         current_time,
                                         mat4(1.0f),
                                         blend_factor);
    }

    void Animator::calculate_blended_bone_transform(SkeletalAnimation *base_animation,
                                                    const AssNodeData *base_node,
                                                    SkeletalAnimation *layered_animation,
                                                    const AssNodeData *layered_node,
                                                    const f32 current_time_base,
                                                    const f32 current_time_layered,
                                                    const mat4 &parent_transform,
                                                    const f32 blend_factor)
    {
        const str &node_name = base_node->name;

        mat4 node_transform = base_node->transformation;
        auto bone = base_animation->find_bone(node_name);
        if (bone)
        {
            bone->update(current_time_base);
            node_transform = bone->get_local_transform();
        }

        mat4 layered_node_transform = layered_node->transformation;
        bone = layered_animation->find_bone(node_name);
        if (bone)
        {
            bone->update(current_time_layered);
            layered_node_transform = bone->get_local_transform();
        }

        // Blend two matrices
        const quat rot_0 = quat_cast(node_transform);
        const quat rot_1 = quat_cast(layered_node_transform);
        const quat final_rot = slerp(rot_0, rot_1, blend_factor);
        mat4 blended_mat = mat4_cast(final_rot);
        blended_mat[3] = mix(node_transform[3], layered_node_transform[3], blend_factor);

        mat4 global_transform = parent_transform * blended_mat;

        const auto &bone_info_map = base_animation->get_bone_id_map();
        if (bone_info_map.count(node_name))
        {
            i32 index = bone_info_map.at(node_name).id;
            auto offset = bone_info_map.at(node_name).offset;

            final_bone_matrices[index] = global_transform * offset;
        }

        for (u64 i = 0; i < base_node->children.size(); i++)
            calculate_blended_bone_transform(base_animation,
                                             &base_node->children[i],
                                             layered_animation,
                                             &layered_node->children[i],
                                             current_time_base,
                                             current_time_layered,
                                             global_transform,
                                             blend_factor);
    }

    void Animator::calculate_bone_transform(const AssNodeData *node, mat4 parent_transform)
    {
        str node_name = node->name;
        mat4 node_transform = node->transformation;

        Bone *bone = current_animation->find_bone(node_name);

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

    SkeletalAnimation *Animator::get_current_animation()
    {
        return current_animation;
    }

    f32 Animator::get_blend_factor()
    {
        return blend_factor;
    }
};  // namespace bls
