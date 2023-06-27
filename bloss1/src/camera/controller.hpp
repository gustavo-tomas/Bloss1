#pragma once

/**
 * @brief Controller for the perspective camera. Can move, rotate and zoom in/out.
 */

#include "camera/camera.hpp"
#include "core/event.hpp"

const f32 SPEED        = 50.0f;
const f32 SENSITIVITY  = 0.03f;

namespace bls
{
    class CameraController
    {
        public:
            CameraController(f32 speed = SPEED, f32 sensitivity = SENSITIVITY);
            ~CameraController();

            void update(f32 dt);
            void on_mouse_move(const MouseMoveEvent& event);
            void on_mouse_scroll(const MouseScrollEvent& event);

            Camera& get_camera();

        private:
            Camera camera;
            f32 mouse_x, mouse_y;
            f32 speed, sensitivity;
    };
};
