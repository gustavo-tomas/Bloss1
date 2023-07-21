#include "renderer/video_player.hpp"
#include "core/game.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}

// @TODO: decouple from OpenGL
#include <GL/glew.h> // Include glew before glfw
#include <GLFW/glfw3.h>

namespace bls
{
    VideoPlayer::VideoPlayer()
    {
        load_frame("bloss1/assets/videos/mh_pro_skate.mp4");

        shader = Shader::create("video_player_shader", "bloss1/assets/shaders/video_player.vs", "bloss1/assets/shaders/video_player.fs");

        // @TODO: texture with data param
        curr_frame = Texture::create(frame_width, frame_height, ImageFormat::RGBA8,
                                     TextureParameter::ClampToEdge, TextureParameter::ClampToEdge,
                                     TextureParameter::Linear, TextureParameter::Linear);
        glTextureSubImage2D(curr_frame->get_id(), 0, 0, 0, frame_width, frame_height, GL_RGBA, GL_UNSIGNED_BYTE, data);

        quad = std::make_unique<Quad>(Game::get().get_renderer(), true);
    }

    VideoPlayer::~VideoPlayer()
    {
        delete[] data; // @TODO: lol
    }

    void VideoPlayer::load_frame(const str& file_name)
    {
        // Create format context for the file
        AVFormatContext* format_context = avformat_alloc_context();
        if (!format_context)
        {
            std::cerr << "failed to allocate av format context\n";
            exit(1);
        }

        // Open the file
        if (avformat_open_input(&format_context, file_name.c_str(), NULL, NULL) != 0)
        {
            std::cerr << "failed to open video file: " << file_name << "\n";
            exit(1);
        }

        // Find the first valid video stream inside the file
        AVCodecParameters* codec_params;
        const AVCodec* codec;
        i32 video_stream_index = -1;

        for (u32 i = 0; i < format_context->nb_streams; i++)
        {
            codec_params = format_context->streams[i]->codecpar;
            codec = avcodec_find_decoder(codec_params->codec_id);

            if (!codec)
                continue;

            if (codec_params->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                video_stream_index = i;
                break;
            }
        }

        // Create codec context
        auto codec_context = avcodec_alloc_context3(codec);
        if (!codec_context)
        {
            std::cerr << "failed to allocate av codec context\n";
            exit(1);
        }

        if (avcodec_parameters_to_context(codec_context, codec_params) < 0)
        {
            std::cerr << "failed to initialize codec context\n";
            exit(1);
        }

        if (avcodec_open2(codec_context, codec, NULL) < 0)
        {
            std::cerr << "failed to open codec\n";
            exit(1);
        }

        auto frame = av_frame_alloc();
        if (!frame)
        {
            std::cerr << "failed to allocate frame\n";
            exit(1);
        }

        auto packet = av_packet_alloc();
        if (!packet)
        {
            std::cerr << "failed to allocate packet\n";
            exit(1);
        }

        // Read frames
        i32 res;
        char error[64] = { };
        while (av_read_frame(format_context, packet) == 0)
        {
            if (packet->stream_index != video_stream_index)
                continue;

            res = avcodec_send_packet(codec_context, packet);
            if (res < 0)
            {
                std::cerr << "failed to decode packet: " << av_make_error_string(error, 64, res) << "\n";
                exit(1);
            }

            res = avcodec_receive_frame(codec_context, frame);
            if (res == AVERROR(EAGAIN) || res == AVERROR_EOF)
                continue;

            else if (res < 0)
            {
                std::cerr << "failed to decode packet: " << av_make_error_string(error, 64, res) << "\n";
                exit(1);
            }

            av_packet_unref(packet);
            break;
        }

        // Create scaler context
        auto scaler_context = sws_getContext(frame->width, frame->height,
                                             codec_context->pix_fmt,
                                             frame->width, frame->height,
                                             AV_PIX_FMT_RGB0,
                                             SWS_BILINEAR,
                                             NULL,
                                             NULL,
                                             NULL);

        if (!scaler_context)
        {
            std::cerr << "failed to initialize scaler context\n";
            exit(1);
        }

        frame_width = frame->width;
        frame_height = frame->height;
        data = new u8[frame_width * frame_height * 4];

        u8* dest[4] = { data, NULL, NULL, NULL };
        i32 dest_linesize[4] = { static_cast<i32>(frame_width) * 4, 0, 0, 0 };

        // Retrieve colored frame
        sws_scale(scaler_context, frame->data, frame->linesize, 0, frame_height, dest, dest_linesize);

        // Free allocated resources
        sws_freeContext(scaler_context);
        avformat_close_input(&format_context);
        avformat_free_context(format_context);
        av_frame_free(&frame);
        av_packet_free(&packet);
        avcodec_free_context(&codec_context);
    }

    void VideoPlayer::render_frame()
    {
        shader->bind();
        curr_frame->bind(0);
        quad->render();
    }
};
