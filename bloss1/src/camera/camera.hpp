#pragma once

/**
 * @brief Good ol perspective camera. This one can move and rotate in all directions except the roll axis.
 */

#include "math/math.hpp"

// Default camera values
const f32 ZOOM  =  45.0f;
const f32 NEAR  =  1.0f;
const f32 FAR   =  1000.0f;
const f32 YAW   = -90.0f;
const f32 PITCH =  0.0f;

namespace bls
{
    class Camera
    {
        public:
            Camera(vec3 position = vec3(0.0f, 0.0f, 10.0f),
                   vec3 front    = vec3(0.0f, 0.0f, -1.0f),
                   vec3 world_up  = vec3(0.0f, 1.0f,  0.0f),

                   f32 yaw = YAW,
                   f32 pitch = PITCH,

                   f32 zoom  = ZOOM,
                   f32 near  = NEAR,
                   f32 far   = FAR);
            ~Camera();

            // @TODO: Too many get/sets :(
            void set_position(const vec3& position);
            void set_rotation(f32 pitch, f32 yaw);
            void set_zoom(f32 zoom);

            mat4 get_view_matrix();
            mat4 get_projection_matrix(f32 width, f32 height);

            vec3 get_position();
            vec3 get_rotation();
            vec3 get_front();
            vec3 get_right();
            vec3 get_up();
            f32 get_zoom();

        private:
            void update_view_matrix();

            mat4 view_matrix;
            vec3 position;
            vec3 front, right, up, world_up;

            f32 yaw, pitch;

            f32 zoom;
            f32 near, far;
    };
};
