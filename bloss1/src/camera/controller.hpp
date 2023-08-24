#pragma once

/**
 * @brief Controller for the perspective camera. Can move, rotate and zoom in/out.
 */

#include "camera/camera.hpp"
#include "core/event.hpp"
#include "ecs/ecs.hpp"

const f32 SPEED       = 80.0f;
const f32 SENSITIVITY = 0.03f;

namespace bls
{
    class CameraController
    {
        public:
            CameraController(vec3& target_position, vec3& target_rotation, vec3 target_offset,
                             PhysicsObject& target_object,
                             f32 speed = SPEED, f32 sensitivity = SENSITIVITY);
            ~CameraController();

            void update(f32 dt);
            void on_mouse_move(const MouseMoveEvent& event);
            void on_mouse_scroll(const MouseScrollEvent& event);

            Camera& get_camera();

        private:
            void update_keyboard(f32 dt, const vec3& front, const vec3& right, const vec3& up);
            void update_controller(f32 dt, const vec3& front, const vec3& right, const vec3& up);

            Camera* camera;
            f32 mouse_x, mouse_y, zoom;
            f32 speed, sensitivity;

            vec3& target_position;
            vec3& target_rotation;
            vec3 target_offset;
            PhysicsObject& target_object;
    };
};
