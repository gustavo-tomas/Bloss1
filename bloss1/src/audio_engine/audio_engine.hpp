#pragma once

/**
 * @brief Interface for an audio engine.
 */

#include "math/math.hpp"

namespace bls
{
    class AudioEngine
    {
        public:
            virtual ~AudioEngine()
            {
            }

            virtual void load(const str &name, const str &path, bool looping = false) = 0;
            virtual void play(const str &name,
                              const vec3 &position = vec3(0.0f),
                              const vec3 &velocity = vec3(0.0f),
                              f32 volume = 1.0f) = 0;
            virtual void play_dist(const str &name,
                                   const vec3 &position = vec3(0.0f),
                                   const vec3 &velocity = vec3(0.0f),
                                   f32 distance_from_source = 0.0f,
                                   const f32 max_dist = 1000.0f) = 0;
            virtual void stop(const str &name) = 0;
            virtual void stop_all() = 0;
            virtual void fade_to(const str &name, const f32 volume, const f64 time) = 0;
            virtual void set_echo_filter(const str &name, f32 delay, f32 decay) = 0;

            static AudioEngine *create();
    };
};  // namespace bls
