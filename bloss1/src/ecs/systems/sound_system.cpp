#include "ecs/ecs.hpp"
#include "core/game.hpp"

namespace bls
{
    // Play any sound that is set to play
    void sound_system(ECS& ecs, f32 dt)
    {
        auto& audio_engine = Game::get().get_audio_engine();
        for (auto& [id, sounds] : ecs.sounds)
        {
            for (auto& [name, sound] : sounds)
            {
                if (sound->play_now)
                {
                    audio_engine.play(name, vec3(0.0f), vec3(0.0f), sound->volume);
                    sound->play_now = false;
                }
            }
        }
    }
};
