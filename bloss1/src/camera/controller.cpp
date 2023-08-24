#include "camera/controller.hpp"
#include "core/input.hpp"

namespace bls
{
    CameraController::CameraController(vec3& target_position, vec3& target_rotation, vec3 target_offset,
                                       PhysicsObject& target_object,
                                       f32 speed, f32 sensitivity)
        : target_position(target_position), target_rotation(target_rotation), target_object(target_object)
    {
        camera = new Camera(target_position, target_offset, target_rotation.x, target_rotation.y);

        this->target_offset = target_offset;

        this->speed = speed;
        this->sensitivity = sensitivity;

        zoom = camera->get_zoom();
        mouse_x = Input::get_mouse_x();
        mouse_y = Input::get_mouse_y();

        // Register controller callbacks
        EventSystem::register_callback<MouseScrollEvent>(BIND_EVENT_FN(CameraController::on_mouse_scroll));
        EventSystem::register_callback<MouseMoveEvent>(BIND_EVENT_FN(CameraController::on_mouse_move));
    }

    CameraController::~CameraController()
    {
        delete camera;
    }

    void CameraController::update(f32 dt)
    {
        // @TODO: use quaternions?
        // Calculate player direction vectors
        vec3 front =
        {
            cos(radians(target_rotation.y))* cos(radians(target_rotation.x)),
            sin(radians(target_rotation.x)),
            sin(radians(target_rotation.y))* cos(radians(target_rotation.x))
        };
        front = normalize(front);

        vec3 right = normalize(cross(front, { 0.0f, 1.0f, 0.0f }));
        vec3 up    = normalize(cross(right, front));

        update_keyboard(dt, front, right, up);
        update_controller(dt, front, right, up);
    }

    void CameraController::update_keyboard(f32, const vec3& front, const vec3& right, const vec3& up)
    {
        // Define movement mappings
        std::map<u32, vec3> movement_mappings =
        {
            { KEY_W,          front },
            { KEY_S,         -front },
            { KEY_D,          right },
            { KEY_A,         -right },
            { KEY_SPACE,         up },
            { KEY_LEFT_CONTROL, -up }
        };

        // Update speed based on input
        // Don't use dt here - the physics system will multiply the final force by dt on the same frame
        for (const auto& [key, direction] : movement_mappings)
            if (Input::is_key_pressed(key))
                target_object.force += direction * speed;

        camera->set_target_position(target_position);
    }

    void CameraController::update_controller(f32 dt, const vec3& front, const vec3& right, const vec3&)
    {
        // Tolerance to better handle floating point fuckery
        const f32 TOLERANCE = 0.2f;

        // Position
        // -------------------------------------------------------------------------------------------------------------
        auto left_x = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_LEFT_X);
        auto left_y = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_LEFT_Y);

        if (fabs(left_y) >= TOLERANCE)
            target_object.force += front * speed * -left_y;

        if (fabs(left_x) >= TOLERANCE)
            target_object.force += right * speed * left_x;

        camera->set_target_position(target_position);

        // Rotation
        // -------------------------------------------------------------------------------------------------------------
        auto right_x = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_RIGHT_X);
        auto right_y = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_RIGHT_Y);

        f32 x_offset = 0.0f;
        f32 y_offset = 0.0f;

        // Calculate X and Y offsets
        if (fabs(right_x) >= TOLERANCE)
            x_offset = right_x * 10.0f;

        if (fabs(right_y) >= TOLERANCE)
            y_offset = -right_y * 10.0f;

        // Calculate rotation
        f32 pitch = target_rotation.x + y_offset * sensitivity * dt * 250.0f;
        f32 yaw   = target_rotation.y + x_offset * sensitivity * dt * 250.0f;

        pitch = clamp(pitch, -89.0f, 89.0f); // Clamp pitch to avoid flipping
        // yaw = fmod(fmod(yaw, 360.0f) + 360.0f, 360.0f); // @TODO: fix yaw overflow

        // Update target rotation
        target_rotation.x = pitch;
        target_rotation.y = yaw;

        camera->set_target_rotation(target_rotation.x, target_rotation.y);

        // Zoom
        // -------------------------------------------------------------------------------------------------------------
        auto trigger_left  = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_LEFT_TRIGGER);
        auto trigger_right = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_RIGHT_TRIGGER);

        // Normalize trigger value between [0, 2]
        trigger_left  += 1.0f;
        trigger_right += 1.0f;

        if (trigger_left >= TOLERANCE)
            zoom += trigger_left * dt * 15.0f;

        if (trigger_right >= TOLERANCE)
            zoom -= trigger_right * dt * 15.0f;

        zoom = clamp(zoom, 45.0f, 90.0f);

        camera->set_target_zoom(zoom);
    }

    void CameraController::on_mouse_move(const MouseMoveEvent& event)
    {
        // Calculate X and Y offsets
        f32 x_offset = event.x_position - mouse_x;
        f32 y_offset = mouse_y - event.y_position;

        // Rotation is done with callbacks for a more smooth feeling
        f32 pitch = target_rotation.x + y_offset * sensitivity;
        f32 yaw = target_rotation.y + x_offset * sensitivity;

        pitch = clamp(pitch, -89.0f, 89.0f); // Clamp pitch to avoid flipping
        // yaw = fmod(fmod(yaw, 360.0f) + 360.0f, 360.0f); // @TODO: fix yaw overflow

        // Update last mouse values
        mouse_x = event.x_position;
        mouse_y = event.y_position;

        // Update target rotation
        target_rotation.x = pitch;
        target_rotation.y = yaw;

        camera->set_target_rotation(target_rotation.x, target_rotation.y);
    }

    void CameraController::on_mouse_scroll(const MouseScrollEvent& event)
    {
        zoom -= event.y_offset * 5.0f;
        zoom = clamp(zoom, 45.0f, 90.0f);

        camera->set_target_zoom(zoom);
    }

    Camera& CameraController::get_camera()
    {
        return *camera;
    }
};
