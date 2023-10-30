#include "core/game.hpp"
#include "ecs/ecs.hpp"
#include "tools/profiler.hpp"

namespace bls
{
    // Play any sound that is set to play
    void sound_system(ECS &ecs, f32)
    {
        BLS_PROFILE_SCOPE("sound_system");

        auto &audio_engine = Game::get().get_audio_engine();
        for (auto &[id, sounds] : ecs.sounds)
        {
            for (auto &[name, sound] : sounds)
            {
                if (sound->play_now)
                {
                    audio_engine.play(name, vec3(0.0f), vec3(0.0f), sound->volume);
                    sound->play_now = false;
                }
            }
        }
    }
};  // namespace bls
