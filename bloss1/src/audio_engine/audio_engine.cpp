#include "audio_engine/audio_engine.hpp"

#include "audio_engine/soloud/audio_engine.hpp"

namespace bls
{
    AudioEngine *AudioEngine::create()
    {
#ifdef _SOLOUD
        return new SoloudAudioEngine();
#else
        throw std::runtime_error(
            "no valid audio engine "
            "defined");
#endif
    }
};  // namespace bls
