#pragma once

/**
 * @brief @TODO: i dont know where im going with this
 *
 */

#include "math/math.hpp"

#include "soloud/include/soloud.h"
#include "soloud/include/soloud_wav.h"

namespace bls
{
    class AudioEngine
    {
        public:
            AudioEngine();
            ~AudioEngine();

            void load(const str& name, const str& path, bool looping = false);
            void play(const str& name, const vec3& position = vec3(0.0f), const vec3& velocity = vec3(0.0f), f32 volume = 1.0f);

        private:
            SoLoud::Soloud soloud;
            std::map<str, std::unique_ptr<SoLoud::Wav>> audios;
    };
};
