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
            HeightMap(const str& texture_path);
            ~HeightMap();

            void render(const mat4& view, const mat4& projection);

        private:
            u32 num_vert_per_patch, num_patches;

            std::shared_ptr<Shader> shader;
            std::shared_ptr<Texture> texture;
            std::unique_ptr<VertexArray> vao;
            std::unique_ptr<VertexBuffer> vbo;
    };
};  // namespace bls
