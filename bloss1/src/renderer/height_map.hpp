#pragma once

/**
 * @brief @TODO
 */

#include "renderer/buffers.hpp"
#include "renderer/shader.hpp"
#include "renderer/texture.hpp"

namespace bls
{
    class HeightMap
    {
        public:
            HeightMap(u32 width,
                      u32 height,
                      u32 min_tess_level = 4,
                      u32 max_tess_level = 64,
                      f32 min_distance = 20,
                      f32 max_distance = 1000);
            ~HeightMap();

            void render(const mat4& view, const mat4& projection, f32 dt);

            u32 min_tess_level, max_tess_level, noise_algorithm;
            f32 min_distance, max_distance;
            vec2 displacement_multiplier;
            f32 fbm_scale, fbm_height, perlin_scale, perlin_height;
            i32 fbm_octaves;

            std::vector<f32> texture_heights;
            bool toggle_gradient;

        private:
            u32 num_vert_per_patch, num_patches;
            vec2 displacement;

            std::shared_ptr<Shader> shader;
            std::vector<std::shared_ptr<Texture>> texture_layers;
            std::unique_ptr<VertexArray> vao;
            std::unique_ptr<VertexBuffer> vbo;
    };
};  // namespace bls
