#pragma once

/**
 * @brief
 *
 */

#include "renderer/buffers.hpp"
#include "renderer/texture.hpp"
#include "renderer/assimp_utils.hpp"
#include "math/math.hpp"

#include "assimp/scene.h"
#include "assimp/Importer.hpp"

#define MAX_BONE_PER_VERTEX 4
#define MAX_BONE_MATRICES 100

namespace bls
{
    struct AssNodeData
    {
        mat4 transformation;
        str name;
        i32 children_count;
        std::vector<AssNodeData> children;
    };

    struct BoneInfo
    {
        i32 id;
        mat4 offset;
    };

    struct KeyPosition
    {
        vec3 position;
        f32 time_stamp;
    };

    struct KeyRotation
    {
        quat orientation;
        f32 time_stamp;
    };

    struct KeyScale
    {
        vec3 scale;
        f32 time_stamp;
    };

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

    // Mesh
    // -----------------------------------------------------------------------------------------------------------------
    class Mesh
    {
        public:
            Mesh(VertexArray* vao, VertexBuffer* vbo, IndexBuffer* ebo,
                 const std::vector<Vertex>& vertices, const std::vector<u32>& indices, const std::vector<std::shared_ptr<Texture>>& textures)
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
            std::vector<std::shared_ptr<Texture>> textures;
    };

    // Bone
    // -----------------------------------------------------------------------------------------------------------------
    class Bone
    {
        public:
            Bone(const str& name, i32 id, const aiNodeAnim* channel);
            ~Bone();

            void update(f32 animation_time);

            mat4 get_local_transform();
            str get_bone_name();
            i32 get_bone_id();
            i32 get_position_index(f32 animation_time);
            i32 get_rotation_index(f32 animation_time);
            i32 get_scale_index(f32 animation_time);

        private:
            f32 get_scale_factor(f32 last_time_stamp, f32 next_time_stamp, f32 animation_time);
            mat4 interpolate_position(f32 animation_time);
            mat4 interpolate_rotation(f32 animation_time);
            mat4 interpolate_scaling(f32 animation_time);

            std::vector<KeyPosition> positions;
            std::vector<KeyRotation> rotations;
            std::vector<KeyScale> scales;

            i32 num_positions;
            i32 num_rotations;
            i32 num_scalings;

            mat4 local_transform;
            str name;
            i32 id;
    };

    // Animation
    // -----------------------------------------------------------------------------------------------------------------
    class SkeletalAnimation
    {
        public:
            SkeletalAnimation(const aiNode* root, const aiAnimation* animation, std::map<str, BoneInfo>& model_bone_info_map, i32& model_bone_count);
            ~SkeletalAnimation();

            Bone* find_bone(const str& name);
            f32 get_ticks_per_second();
            f32 get_duration();
            f32 get_duration_seconds();
            AssNodeData& get_root_node();
            std::map<str, BoneInfo>& get_bone_id_map();

        private:
            void read_missing_bones(const aiAnimation* animation, std::map<str, BoneInfo>& model_bone_info_map, i32& model_bone_count);
            void read_hierarchy_data(AssNodeData& dest, const aiNode* root);

            f32 duration;
            i32 ticks_per_second;
            std::vector<Bone*> bones;
            AssNodeData root_node;
            std::map<str, BoneInfo> bone_info_map;
    };

    // Animator
    // -----------------------------------------------------------------------------------------------------------------
    class Animator
    {
        public:
            Animator(SkeletalAnimation* animation);
            ~Animator();

            void update(f32 dt);
            void play(SkeletalAnimation* animation);
            void blend_animations(SkeletalAnimation* base_animation, SkeletalAnimation* layered_animation, f32 blend_factor, f32 dt);
            void calculate_bone_transform(const AssNodeData* node, mat4 parent_transform);
            void calculate_blended_bone_transform(SkeletalAnimation* base_animation, const AssNodeData* base_node,
                                                  SkeletalAnimation* layered_animation, const AssNodeData* layered_node,
                                                  const f32 current_time_base, const f32 current_time_layered,
                                                  const mat4& parent_transform,
                                                  const f32 blend_factor);
            std::vector<mat4> get_final_bone_matrices();
            SkeletalAnimation* get_current_animation();

        private:
            std::vector<mat4> final_bone_matrices;
            SkeletalAnimation* current_animation;
            f32 current_time;
            f32 current_time_base = 0.0f;
            f32 current_time_layered = 0.0f;
    };

    // Model
    // -----------------------------------------------------------------------------------------------------------------
    class Model
    {
        public:
            Model(const str& path, bool flip_uvs);
            ~Model();

            static std::shared_ptr<Model> create(const str& name, const str& path, bool flip_uvs); // i know i know

            str path;
            bool flip_uvs;
            std::vector<Mesh*> meshes;
            std::map<str, BoneInfo> bone_info_map;
            std::map<str, std::unique_ptr<SkeletalAnimation>> animations;
            std::unique_ptr<Animator> animator;
            i32 bone_counter;

        private:

            // Helper methods
            void process_node(aiNode* node, const aiScene* scene);
            Mesh* process_mesh(aiMesh* mesh, const aiScene* scene);
            std::vector<std::shared_ptr<Texture>> load_material_textures(aiMaterial* mat, aiTextureType type);

            auto& get_bone_info_map();
            i32& get_bone_count();

            void set_vertex_bone_data_to_default(Vertex& vertex);
            void set_vertex_bone_data(Vertex& vertex, i32 bone_id, f32 weight);
            void extract_bone_weight_for_vertices(std::vector<Vertex>& vertices, aiMesh* mesh);

            Assimp::Importer* importer;
    };
};
