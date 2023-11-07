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
            HeightMap(const str& texture_path,
                      u32 min_tess_level = 4,
                      u32 max_tess_level = 64,
                      f32 min_distance = 20,
                      f32 max_distance = 1000);
            ~HeightMap();

            void render(const mat4& view, const mat4& projection);

            u32 min_tess_level, max_tess_level;
            f32 min_distance, max_distance;

        private:
            u32 num_vert_per_patch, num_patches;

            std::shared_ptr<Shader> shader;
            std::shared_ptr<Texture> texture;
            std::unique_ptr<VertexArray> vao;
            std::unique_ptr<VertexBuffer> vbo;
    };
};  // namespace bls
