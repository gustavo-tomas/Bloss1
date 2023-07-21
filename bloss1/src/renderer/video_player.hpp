#pragma once

/**
 * @brief @TODO
 *
 */

#include "renderer/texture.hpp"
#include "renderer/shader.hpp"
#include "renderer/primitives/quad.hpp"

namespace bls
{
    class VideoPlayer
    {
        public:
            VideoPlayer();
            ~VideoPlayer();

            void render_frame();

        private:
            void load_frame(const str& file_name);

            std::unique_ptr<Quad> quad;
            std::shared_ptr<Texture> curr_frame;
            std::shared_ptr<Shader> shader;

            u8* data;
            u32 frame_width, frame_height;
    };
};
