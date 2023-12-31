#pragma once

/**
 * @brief Soloud backend for the audio engine.
 */

#include "audio_engine/audio_engine.hpp"
#include "soloud/include/soloud.h"
#include "soloud/include/soloud_filter.h"
#include "soloud/include/soloud_wav.h"

namespace bls
{
    class SoloudAudioEngine : public AudioEngine
    {
        public:
            SoloudAudioEngine();
            ~SoloudAudioEngine();

            void load(const str &name, const str &path, bool looping = false) override;
            void play(const str &name,
                      const vec3 &position = vec3(0.0f),
                      const vec3 &velocity = vec3(0.0f),
                      f32 volume = 1.0f) override;
            void play_dist(const str &name,
                           const vec3 &position = vec3(0.0f),
                           const vec3 &velocity = vec3(0.0f),
                           f32 distance_from_source = 0.0f,
                           const f32 max_dist = 1000.0f) override;
            void stop(const str &name) override;
            void stop_all() override;

            void fade_to(const str &name, const f32 volume, const f64 time) override;
            void set_echo_filter(const str &name, f32 delay, f32 decay) override;

        private:
            SoLoud::Soloud soloud;
            std::map<str, SoLoud::handle> handles;
            std::map<str, std::unique_ptr<SoLoud::Wav>> audios;
            std::map<str, std::unique_ptr<SoLoud::Filter>> filters;
    };
};  // namespace bls
