#pragma once

/**
 * @brief Soloud backend for the audio engine.
 */

#include "audio_engine/audio_engine.hpp"

#include "soloud/include/soloud.h"
#include "soloud/include/soloud_wav.h"
#include "soloud/include/soloud_filter.h"

namespace bls
{
    class SoloudAudioEngine : public AudioEngine
    {
        public:
            SoloudAudioEngine();
            ~SoloudAudioEngine();

            void load(const str& name, const str& path, bool looping = false) override;
            void play(const str& name,
                      const vec3& position = vec3(0.0f),
                      const vec3& velocity = vec3(0.0f),
                      f32 volume = 1.0f) override;

            void set_echo_filter(const str& name, f32 delay, f32 decay) override;

        private:
            SoLoud::Soloud soloud;
            std::map<str, std::unique_ptr<SoLoud::Wav>> audios;
            std::map<str, std::unique_ptr<SoLoud::Filter>> filters;
    };
};
