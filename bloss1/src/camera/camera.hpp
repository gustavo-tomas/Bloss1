#pragma once

/**
 * @brief Good ol perspective camera. This one can move and rotate in all directions except the roll axis.
 */

#include "math/math.hpp"

namespace bls
{
    class Camera
    {
        public:
            Camera(vec3 target_position = vec3(0.0f),
                   vec3 target_offset = vec3(0.0f),

                   f32 target_pitch = 0.0f,
                   f32 target_yaw = 0.0f,

                   vec3 world_up = vec3(0.0f, 1.0f, 0.0f),

                   f32 zoom = 45.0f,
                   f32 near = 1.0f,
                   f32 far  = 1000.0f,
                   f32 lerp_factor = 7.5f);
            ~Camera();

            void update(f32 dt);

            // @TODO: Too many get/sets :(
            void set_target_position(const vec3& position);
            void set_target_rotation(f32 pitch, f32 yaw);
            void set_target_offset(const vec3& offset);
            void set_target_zoom(f32 zoom);

            mat4 get_view_matrix();
            mat4 get_projection_matrix(f32 width, f32 height);

            vec3 get_position();

            f32 get_zoom();
            f32 get_near();
            f32 get_far();

        private:
            mat4 view_matrix;
            vec3 position, front, right, up, world_up;

            vec3 target_position, target_offset;
            f32 target_pitch, target_yaw, target_zoom;

            f32 zoom, near, far;
            f32 lerp_factor;
    };
};
