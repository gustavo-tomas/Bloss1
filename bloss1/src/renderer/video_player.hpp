#pragma once

/**
 * @brief @TODO
 *
 */

#include "renderer/texture.hpp"
#include "renderer/shader.hpp"
#include "renderer/primitives/quad.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

namespace bls
{
    class VideoPlayer
    {
        public:
            VideoPlayer(const str& file);
            ~VideoPlayer();

            void render(f32 dt);
            bool finished();

        private:
            void open_file(const str& file);
            void read_frame();
            void render_frame();

            std::unique_ptr<Quad> quad;
            std::shared_ptr<Texture> curr_frame;
            std::shared_ptr<Shader> shader;

            u32 frame_width, frame_height;

            u8* data;
            u8* dest[4];
            i32 dest_linesize[4];

            AVFormatContext* format_context = NULL;
            AVCodecParameters* codec_params = NULL;
            const AVCodec* codec = NULL;
            AVCodecContext* codec_context = NULL;
            AVFrame* frame = NULL;
            AVPacket* packet = NULL;
            SwsContext* scaler_context = NULL;
            i32 video_stream_index = -1;

            bool read_frames;
    };
};
