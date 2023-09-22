#include "audio_engine.hpp"
#include "soloud/include/soloud_echofilter.h"

namespace bls
{
    AudioEngine::AudioEngine()
    {
        // Initialize SoLoud (automatic back-end selection)
        // also, enable visualization for FFT calc
        soloud.init();
        soloud.setVisualizationEnable(1);
    }

    AudioEngine::~AudioEngine()
    {
        // Clean up SoLoud
        soloud.deinit();
    }

    void AudioEngine::load(const str& name, const str& path, bool looping)
    {
        if (audios.count(name))
        {
            std::cerr << "audio '" << name << "' is already loaded\n";
            return;
        }

        // Load a wav/ogg/mp3 file
        SoLoud::Wav* wav = new SoLoud::Wav();

        auto res = wav->load(path.c_str());
        if (res != SoLoud::SOLOUD_ERRORS::SO_NO_ERROR)
            std::runtime_error("failed to load audio: '" + path + "'");

        wav->setLooping(looping);

        audios[name] = std::unique_ptr<SoLoud::Wav>(wav);
    }

    void AudioEngine::play(const str& name, const vec3& position, const vec3& velocity, f32 volume)
    {
        // This returns a handle
        soloud.play3d(*audios[name],
                      position.x, position.y, position.z,
                      velocity.x, velocity.y, velocity.z,
                      volume);

        // More useful methods
        // soloud.setVolume(handle, 0.15f);           // Set volume; 1.0f is "normal"
        // soloud.setPan(handle, -0.2f);              // Set pan; -1 is left, 1 is right
        // soloud.setRelativePlaySpeed(handle, 0.7f); // Play a bit slower; 1.0f is normal
    }

    void AudioEngine::set_echo_filter(const str& name, f32 delay, f32 decay)
    {
        if (!filters.count(name))
            filters[name] = std::make_unique<SoLoud::EchoFilter>();

        static_cast<SoLoud::EchoFilter*>(filters[name].get())->setParams(delay, decay);
        audios[name]->setFilter(0, filters[name].get());
    }
};
