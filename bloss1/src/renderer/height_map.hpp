#pragma once

/**
 * @brief @TODO
 */

#include "config.hpp"
#include "core/game.hpp"
#include "core/logger.hpp"
#include "renderer/buffers.hpp"
#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "renderer/texture.hpp"
#include "stb/stb_image.h"

namespace bls
{
    class HeightMap
    {
        public:
            HeightMap(const str& texture_path);
            ~HeightMap();

            void render(const mat4& view, const mat4& projection);

        private:
            const str texture_path;
            i32 width, height, channels;
            u32 num_strips, num_triangles_per_strip;

            std::shared_ptr<Shader> shader;
            std::unique_ptr<VertexArray> vao;
            std::unique_ptr<VertexBuffer> vbo;
            std::unique_ptr<IndexBuffer> ebo;
    };
};  // namespace bls
