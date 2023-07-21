#include "renderer/video_player.hpp"
#include "core/game.hpp"

namespace bls
{
    VideoPlayer::VideoPlayer(const str& file)
    {
        open_file(file);

        shader = Shader::create("video_player_shader", "bloss1/assets/shaders/video_player.vs", "bloss1/assets/shaders/video_player.fs");

        curr_frame = Texture::create(frame_width, frame_height, ImageFormat::RGBA8,
                                     TextureParameter::ClampToEdge, TextureParameter::ClampToEdge,
                                     TextureParameter::Linear, TextureParameter::Linear);

        quad = std::make_unique<Quad>(Game::get().get_renderer(), true);

        read_frames = true;
    }

    VideoPlayer::~VideoPlayer()
    {
        // Free allocated resources
        avformat_close_input(&format_context);
        avformat_free_context(format_context);
        avcodec_free_context(&codec_context);
        sws_freeContext(scaler_context);
        av_frame_free(&frame);
        av_packet_free(&packet);

        delete[] data;

        std::cout << "video player destroyed successfully\n";
    }

    void VideoPlayer::render(f32 dt)
    {
        if (finished())
            return;

        read_frame();
        render_frame();
    }

    void VideoPlayer::open_file(const str& file)
    {
        // Create format context for the file
        format_context = avformat_alloc_context();
        if (!format_context)
        {
            std::cerr << "failed to allocate av format context\n";
            exit(1);
        }

        // Open the file
        if (avformat_open_input(&format_context, file.c_str(), NULL, NULL) != 0)
        {
            std::cerr << "failed to open video file: " << file << "\n";
            exit(1);
        }

        // Find the first valid video stream inside the file
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
        codec_context = avcodec_alloc_context3(codec);
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

        // Allocate frame
        frame = av_frame_alloc();
        if (!frame)
        {
            std::cerr << "failed to allocate frame\n";
            exit(1);
        }

        // Allocate packet
        packet = av_packet_alloc();
        if (!packet)
        {
            std::cerr << "failed to allocate packet\n";
            exit(1);
        }

        // Set frame dimensions
        frame_width = codec_params->width;
        frame_height = codec_params->height;

        // Allocate dest data
        for (u32 i = 0; i < 4; i++)
        {
            dest[i] = 0;
            dest_linesize[i] = 0;
        }

        data = new u8[frame_width * frame_height * 4];
        dest[0] = data;
        dest_linesize[0] = frame_width * 4;
    }

    void VideoPlayer::read_frame()
    {
        i32 res;
        char error[64] = { };

        while (read_frames)
        {
            if (av_read_frame(format_context, packet) < 0)
            {
                read_frames = false;
                break;
            }

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
        if (!scaler_context)
        {
            scaler_context = sws_getContext(frame->width, frame->height,
                                            codec_context->pix_fmt,
                                            frame->width, frame->height,
                                            AV_PIX_FMT_RGB0,
                                            SWS_BILINEAR, NULL, NULL, NULL);

            if (!scaler_context)
            {
                std::cerr << "failed to initialize scaler context\n";
                exit(1);
            }
        }

        // Retrieve colored frame
        sws_scale(scaler_context, frame->data, frame->linesize, 0, frame_height, dest, dest_linesize);

        // Update texture data
        curr_frame->set_data(data);
    }

    void VideoPlayer::render_frame()
    {
        shader->bind();
        curr_frame->bind(0);
        quad->render();
    }

    bool VideoPlayer::finished()
    {
        return !read_frames;
    }
};
