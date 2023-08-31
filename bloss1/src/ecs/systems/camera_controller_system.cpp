#include "ecs/ecs.hpp"
#include "core/input.hpp"

namespace bls
{
    void update_keyboard(ECS& ecs, u32 id, const vec3& front, const vec3& right, const vec3& up, f32 dt);
    void update_controller(ECS& ecs, u32 id, const vec3& front, const vec3& right, const vec3& up, f32 dt);

    void camera_controller_system(ECS& ecs, f32 dt)
    {
        // Update all controllers
        auto& camera_controllers = ecs.camera_controllers;
        auto& transforms = ecs.transforms;
        for (const auto& [id, controller] : camera_controllers)
        {
            auto transform = transforms[id].get();

            // @TODO: use quaternions?
            // Calculate target direction vectors
            vec3 front =
            {
                cos(radians(transform->rotation.y))* cos(radians(transform->rotation.x)),
                sin(radians(transform->rotation.x)),
                sin(radians(transform->rotation.y))* cos(radians(transform->rotation.x))
            };
            front = normalize(front);

            vec3 right = normalize(cross(front, { 0.0f, 1.0f, 0.0f }));
            vec3 up    = normalize(cross(right, front));

            update_keyboard(ecs, id, front, right, up, dt);
            update_controller(ecs, id, front, right, up, dt);
        }
    }

    void update_keyboard(ECS& ecs, u32 id, const vec3& front, const vec3& right, const vec3& up, f32)
    {
        auto object = ecs.physics_objects[id].get();
        auto controller = ecs.camera_controllers[id].get();
        auto transform = ecs.transforms[id].get();

        // Position
        // -------------------------------------------------------------------------------------------------------------
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
                object->force += direction * controller->speed;

        // Rotation
        // -------------------------------------------------------------------------------------------------------------
        auto [curr_mouse_x, curr_mouse_y] = Input::get_mouse_position();

        // Calculate X and Y offsets
        f32 x_offset = curr_mouse_x - controller->mouse_x;
        f32 y_offset = controller->mouse_y - curr_mouse_y;

        // Calculate rotation
        f32 pitch = transform->rotation.x + y_offset * controller->sensitivity;
        f32 yaw = transform->rotation.y + x_offset * controller->sensitivity;

        pitch = clamp(pitch, -89.0f, 89.0f); // Clamp pitch to avoid flipping
        // yaw = fmod(fmod(yaw, 360.0f) + 360.0f, 360.0f); // @TODO: fix yaw overflow

        // Update last mouse values
        controller->mouse_x = curr_mouse_x;
        controller->mouse_y = curr_mouse_y;

        // Update target rotation
        transform->rotation.x = pitch;
        transform->rotation.y = yaw;
    }

    void update_controller(ECS& ecs, u32 id, const vec3& front, const vec3& right, const vec3&, f32 dt)
    {
        auto object = ecs.physics_objects[id].get();
        auto controller = ecs.camera_controllers[id].get();
        auto camera = ecs.cameras[id].get();
        auto transform = ecs.transforms[id].get();

        // Tolerance to better handle floating point fuckery
        const f32 TOLERANCE = 0.2f;

        // Position
        // -------------------------------------------------------------------------------------------------------------
        auto left_x = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_LEFT_X);
        auto left_y = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_LEFT_Y);

        if (fabs(left_y) >= TOLERANCE)
            object->force += front * controller->speed * -left_y;

        if (fabs(left_x) >= TOLERANCE)
            object->force += right * controller->speed * left_x;

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
        f32 pitch = transform->rotation.x + y_offset * controller->sensitivity * dt * 250.0f;
        f32 yaw   = transform->rotation.y + x_offset * controller->sensitivity * dt * 250.0f;

        pitch = clamp(pitch, -89.0f, 89.0f); // Clamp pitch to avoid flipping
        // yaw = fmod(fmod(yaw, 360.0f) + 360.0f, 360.0f); // @TODO: fix yaw overflow

        // Update target rotation
        transform->rotation.x = pitch;
        transform->rotation.y = yaw;

        // Zoom
        // -------------------------------------------------------------------------------------------------------------
        auto trigger_left  = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_LEFT_TRIGGER);
        auto trigger_right = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_RIGHT_TRIGGER);

        // Normalize trigger value between [0, 2]
        trigger_left  += 1.0f;
        trigger_right += 1.0f;

        if (trigger_left >= TOLERANCE)
            camera->target_zoom += trigger_left * dt * 15.0f;

        if (trigger_right >= TOLERANCE)
            camera->target_zoom -= trigger_right * dt * 15.0f;

        camera->target_zoom = clamp(camera->target_zoom, 45.0f, 90.0f);
    }
};
