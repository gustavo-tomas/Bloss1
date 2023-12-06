#include "audio_engine/soloud/audio_engine.hpp"

#include "core/logger.hpp"
#include "soloud/include/soloud_echofilter.h"

namespace bls
{
    SoloudAudioEngine::SoloudAudioEngine()
    {
        // Initialize SoLoud (automatic back-end selection)
        // also, enable visualization for FFT calc
        soloud.init();
        soloud.setVisualizationEnable(1);
    }

    SoloudAudioEngine::~SoloudAudioEngine()
    {
        // Clean up SoLoud
        soloud.deinit();
    }

    void SoloudAudioEngine::load(const str &name, const str &path, bool looping)
    {
        if (audios.count(name))
        {
            LOG_WARNING("audio '%s' is already loaded", name.c_str());
            return;
        }

        // Load a wav/ogg/mp3 file
        SoLoud::Wav *wav = new SoLoud::Wav();

        auto res = wav->load(path.c_str());
        if (res != SoLoud::SOLOUD_ERRORS::SO_NO_ERROR) throw std::runtime_error("failed to load audio: '" + path + "'");

        wav->setLooping(looping);

        audios[name] = std::unique_ptr<SoLoud::Wav>(wav);
    }

    void SoloudAudioEngine::play(const str &name, const vec3 &position, const vec3 &velocity, f32 volume)
    {
        // This returns a handle
        auto handle = soloud.play3d(
            *audios[name], position.x, position.y, position.z, velocity.x, velocity.y, velocity.z, volume);

        if (handles.count(name) == 0) handles[name] = handle;
    }

    void SoloudAudioEngine::play_dist(
        const str &name, const vec3 &position, const vec3 &velocity, f32 distance_from_source, const f32 max_dist)
    {
        f32 distance = clamp(distance_from_source, 0.0f, max_dist);

        const f32 MIN_VOLUME = 0.001f;
        f32 volume = 1.0f - (distance / max_dist);
        volume = clamp(volume, MIN_VOLUME, 1.0f);

        play(name, position, velocity, volume);
    }

    void SoloudAudioEngine::stop(const str &name)
    {
        soloud.stop(handles[name]);
    }

    void SoloudAudioEngine::stop_all()
    {
        soloud.stopAll();
    }

    void SoloudAudioEngine::fade_to(const str &name, const f32 volume, const f64 time)
    {
        soloud.fadeVolume(handles[name], volume, time);
    }

    void SoloudAudioEngine::set_echo_filter(const str &name, f32 delay, f32 decay)
    {
        if (!filters.count(name)) filters[name] = std::make_unique<SoLoud::EchoFilter>();

        static_cast<SoLoud::EchoFilter *>(filters[name].get())->setParams(delay, decay);
        audios[name]->setFilter(0, filters[name].get());
    }
};  // namespace bls
