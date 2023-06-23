#include "camera/controller.hpp"
#include "core/input.hpp"

namespace bls
{
    CameraController::CameraController(f32 speed, f32 sensitivity)
    {
        camera = Camera();

        this->speed = speed;
        this->sensitivity = sensitivity;

        mouse_x = Input::get_mouse_x();
        mouse_y = Input::get_mouse_y();

        // Register controller callbacks
        EventSystem::register_callback<MouseScrollEvent>(BIND_EVENT_FN(CameraController::on_mouse_scroll));
        EventSystem::register_callback<MouseMoveEvent>(BIND_EVENT_FN(CameraController::on_mouse_move));
    }

    CameraController::~CameraController()
    {

    }

    void CameraController::update(f32 dt)
    {
        f32 velocity = speed * dt;

        auto position = camera.get_position();
        auto front = camera.get_front();
        auto right = camera.get_right();
        auto up = camera.get_up();

        // Forward
        if (Input::is_key_pressed(KEY_W))
            position += front * velocity;

        // Backward
        if (Input::is_key_pressed(KEY_S))
            position -= front * velocity;

        // Right
        if (Input::is_key_pressed(KEY_D))
            position += right * velocity;

        // Left
        if (Input::is_key_pressed(KEY_A))
            position -= right * velocity;

        // Up
        if (Input::is_key_pressed(KEY_SPACE))
            position += up * velocity;

        // Down
        if (Input::is_key_pressed(KEY_LEFT_CONTROL))
            position -= up * velocity;

        camera.set_position(position);
    }

    void CameraController::on_mouse_move(const MouseMoveEvent& event)
    {
        // X and Y offsets are inverted
        f32 x_offset = event.x_position - mouse_x;
        f32 y_offset = mouse_y - event.y_position;

        // Rotation is done with callbacks for a more smooth feeling
        auto rotation = camera.get_rotation();
        f32 pitch = rotation.x + y_offset * sensitivity;
        f32 yaw = rotation.y + x_offset * sensitivity;

        // Constrain pitch to avoid flipping
        pitch = clamp(pitch, -89.0f, 89.0f);

        // Update last mouse values
        mouse_x = event.x_position;
        mouse_y = event.y_position;

        camera.set_rotation(pitch, yaw);
    }

    void CameraController::on_mouse_scroll(const MouseScrollEvent& event)
    {
        f32 zoom = camera.get_zoom() - event.y_offset;
        zoom = clamp(zoom, 1.0f, 45.0f);

        camera.set_zoom(zoom);
    }

    Camera& CameraController::get_camera()
    {
        return camera;
    }
};
