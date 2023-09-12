#pragma once

/**
 * @brief @TODO: same issues as the skybox
 *
 */

#include "renderer/shader.hpp"
#include "renderer/primitives/quad.hpp"
#include "ecs/ecs.hpp"

namespace bls
{
    class ShadowMap
    {
        public:
            ShadowMap(Camera& camera, const vec3& light_dir);
            ~ShadowMap();

            void bind();
            void unbind();
            void bind_maps(Shader& shader, u32 slot);

            void set_light_dir(const vec3& light_dir);
            vec3 get_light_dir();
            Shader& get_shadow_depth_shader() const;
            void render_debug();

        private:
            std::vector<vec4> get_frustum_corners_world_space(const mat4& projview);
            std::vector<vec4> get_frustum_corners_world_space(const mat4& proj, const mat4& view);
            mat4 get_light_space_matrix(f32 near, f32 far);
            std::vector<mat4> get_light_space_matrices();

            Camera& camera;

            std::shared_ptr<Shader> shadow_map_depth;
            std::shared_ptr<Shader> debug_depth;

            vec3 light_dir;

            std::vector<f32> shadow_cascade_levels;
            u32 light_FBO, matrices_UBO;
            u32 light_depth_maps;
            u32 depth_map_resolution;
            std::unique_ptr<Quad> debug_quad;
    };
};
