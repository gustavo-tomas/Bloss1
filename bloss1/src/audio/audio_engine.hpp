#pragma once

/**
 * @brief @TODO: i dont know where im going with this
 *
 */

#include "math/math.hpp"

#include "soloud/include/soloud.h"
#include "soloud/include/soloud_wav.h"
#include "soloud/include/soloud_filter.h"

namespace bls
{
    class AudioEngine
    {
        public:
            AudioEngine();
            ~AudioEngine();

            void load(const str& name, const str& path, bool looping = false);
            void play(const str& name, const vec3& position = vec3(0.0f), const vec3& velocity = vec3(0.0f), f32 volume = 1.0f);

            void set_echo_filter(const str& name, f32 delay, f32 decay);

        private:
            SoLoud::Soloud soloud;
            std::map<str, std::unique_ptr<SoLoud::Wav>> audios;
            std::map<str, std::unique_ptr<SoLoud::Filter>> filters;
    };
};
