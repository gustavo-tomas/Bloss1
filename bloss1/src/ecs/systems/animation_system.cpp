#include "ecs/ecs.hpp"
#include "tools/profiler.hpp"

namespace bls
{
    void animation_system(ECS &ecs, f32 dt)
    {
        BLS_PROFILE_SCOPE("animation_system");

        auto &animations = ecs.transform_animations;
        auto &transforms = ecs.transforms;
        auto &timers = ecs.timers;
        for (const auto &[id, animation] : animations)
        {
            auto &key_frames = animation->key_frames;
            auto &curr_frame_idx = animation->curr_frame_idx;
            auto curr_frame = key_frames[curr_frame_idx];
            auto transform = transforms[id].get();
            auto timer = timers[id].get();

            // Set transform as curr frame transform
            if (timer->time < curr_frame.duration)
            {
                f32 interpolation_factor = dt / (curr_frame.duration - timer->time);
                transform->position = mix(transform->position, curr_frame.transform.position, interpolation_factor);
                transform->rotation = mix(transform->rotation, curr_frame.transform.rotation, interpolation_factor);
                transform->scale = mix(transform->scale, curr_frame.transform.scale, interpolation_factor);
            }

            // Update curr frame when frame duration ends
            timer->time += dt;
            if (timer->time > curr_frame.duration)
            {
                timer->time = 0.0f;
                curr_frame_idx = (curr_frame_idx + 1) % key_frames.size();
            }
        }
    }
};  // namespace bls
