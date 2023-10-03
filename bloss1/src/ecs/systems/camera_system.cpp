#include "ecs/ecs.hpp"
#include "core/game.hpp"
#include "tools/profiler.hpp"

namespace bls
{
    void camera_system(ECS& ecs, f32 dt)
    {
        BLS_PROFILE_SCOPE("camera_system");

        auto width  = static_cast<f32>(Game::get().get_window().get_width());
        auto height = static_cast<f32>(Game::get().get_window().get_height());

        // Update all cameras
        auto& cameras = ecs.cameras;
        auto& transforms = ecs.transforms;
        for (const auto& [id, camera] : cameras)
        {
            auto transform = transforms[id].get();

            auto world_up = camera->world_up;
            auto target_offset = camera->target_offset;
            auto target_zoom = camera->target_zoom;

            auto target_position = transform->position;
            auto target_yaw = transform->rotation.y;
            auto target_pitch = transform->rotation.x;

            // Update target values
            auto target_front = vec3(cos(radians(target_yaw)) * cos(radians(target_pitch)),
                                     sin(radians(target_pitch)),
                                     sin(radians(target_yaw)) * cos(radians(target_pitch)));
            target_front = normalize(target_front);

            auto target_right = normalize(cross(target_front, world_up));
            auto target_up    = normalize(cross(target_right, target_front));

            target_position = target_position + target_front * (-target_offset.z);
            target_position = target_position + target_up * target_offset.y;
            target_position = target_position + target_right * target_offset.x;

            auto& cam_position = camera->position;
            auto& cam_front = camera->front;
            auto& cam_up = camera->up;
            auto& cam_zoom = camera->zoom;
            auto& cam_near = camera->near;
            auto& cam_far = camera->far;

            auto& view_matrix = camera->view_matrix;
            auto& projection_matrix = camera->projection_matrix;

            // Update camera values
            f32 delta_lerp = clamp(camera->lerp_factor * dt, 0.0f, 1.0f);
            cam_position = mix(cam_position, target_position, delta_lerp);
            cam_front    = mix(cam_front, target_front, delta_lerp);
            cam_up       = mix(cam_up, target_up, delta_lerp);
            cam_zoom     = mix(cam_zoom, target_zoom, delta_lerp);

            // Update view and projection matrices
            view_matrix = look_at(cam_position, cam_position + cam_front, camera->up);
            projection_matrix = perspective(radians(cam_zoom), width / height, cam_near, cam_far);
        }
    }
};
