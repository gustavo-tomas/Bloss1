#include "core/game.hpp"
#include "core/input.hpp"
#include "ecs/ecs.hpp"
#include "ecs/entities.hpp"
#include "renderer/model.hpp"
#include "tools/profiler.hpp"

namespace bls
{
    // Constants
    const f32 TOLERANCE = 0.2f;  // Tolerance to better handle floating point fuckery
    const vec3 WORLD_UP = vec3(0.0f, 1.0f, 0.0f);

    const f32 MIN_CAMERA_ZOOM = 45.0f;
    const f32 MAX_CAMERA_ZOOM = 70.0f;

    const f32 MIN_PLAYER_PITCH = -89.0f;  // 0.0f
    const f32 MAX_PLAYER_PITCH = 89.0f;   // 25.0f

    const f32 PLAYER_TIMER_JUMP = 200000.0f;
    const f32 PLAYER_TIMER_JUMP_COOLDOWN = 0.0f;
    const f32 PLAYER_TIMER_SHOOT_COOLDOWN = 1.0f;

    const str PLAYER_TIMER_STR_JUMP = "jumping";
    const str PLAYER_TIMER_STR_JUMP_COOLDOWN = "jump_cooldown";
    const str PLAYER_TIMER_STR_SHOOT_COOLDOWN = "shoot_cooldown";
    const str PLAYER_TIMER_STR_SHOOT_ANIMATION = "shoot_animation";

    std::map<str, f32> player_timers = {{PLAYER_TIMER_STR_JUMP, 0.0f},
                                        {PLAYER_TIMER_STR_JUMP_COOLDOWN, 0.0f},
                                        {PLAYER_TIMER_STR_SHOOT_ANIMATION, 0.0f},
                                        {PLAYER_TIMER_STR_SHOOT_COOLDOWN, PLAYER_TIMER_SHOOT_COOLDOWN}};

    bool player_shooting = false;

    void update_keyboard(ECS &ecs, u32 id, const vec3 &front, const vec3 &right, const vec3 &up, f32 dt);
    void update_controller(ECS &ecs, u32 id, const vec3 &front, const vec3 &right, const vec3 &up, f32 dt);
    void shoot(ECS &ecs, const Transform &transform, const PhysicsObject &object);

    void player_controller_system(ECS &ecs, f32 dt)
    {
        BLS_PROFILE_SCOPE("player_controller_system");

        // Update all controllers
        auto &camera_controllers = ecs.camera_controllers;
        auto &transforms = ecs.transforms;
        for (const auto &[id, controller] : camera_controllers)
        {
            auto transform = transforms[id].get();

            // Calculate target direction vectors without vertical influence
            vec3 front = {cos(radians(transform->rotation.y)) * cos(radians(transform->rotation.x)),
                          sin(radians(transform->rotation.x)),
                          sin(radians(transform->rotation.y)) * cos(radians(transform->rotation.x))};
            front = normalize(front);

            vec3 right = normalize(cross(front, {0.0f, 1.0f, 0.0f}));
            vec3 up = normalize(cross(right, front));

            // update_keyboard(ecs, id, front, right, up, dt);   // Keyboard for debugging purposes
            update_controller(ecs, id, front, right, up, dt);  // Controller is the actual player controller

            // Update hitpoints
            if (ecs.names[id] == "player" && ecs.texts.count(id))
                ecs.texts[id]->text = to_str(static_cast<u32>(ecs.hitpoints[id]));
        }
    }

    void update_keyboard(ECS &ecs, u32 id, const vec3 &front, const vec3 &right, const vec3 &, f32)
    {
        auto object = ecs.physics_objects[id].get();
        auto controller = ecs.camera_controllers[id].get();
        auto transform = ecs.transforms[id].get();

        // Position
        // -------------------------------------------------------------------------------------------------------------
        // Define movement mappings
        std::map<u32, std::pair<vec3, f32>> movement_mappings = {{KEY_W, {front, controller->speed.z}},
                                                                 {KEY_S, {-front, controller->speed.z}},
                                                                 {KEY_D, {right, controller->speed.x}},
                                                                 {KEY_A, {-right, controller->speed.x}},
                                                                 {KEY_SPACE, {WORLD_UP, controller->speed.y}}};

        // Update speed based on input
        // Don't use dt here - the physics system will multiply the final force by dt on the same frame
        for (const auto &[key, mapping] : movement_mappings)
            if (Input::is_key_pressed(key)) object->force += mapping.first * mapping.second;

        // Rotation
        // -------------------------------------------------------------------------------------------------------------
        auto [curr_mouse_x, curr_mouse_y] = Input::get_mouse_position();

        // Calculate X and Y offsets
        f32 x_offset = curr_mouse_x - controller->mouse_x;
        f32 y_offset = controller->mouse_y - curr_mouse_y;

        // Calculate rotation
        f32 pitch = transform->rotation.x + y_offset * controller->sensitivity;
        f32 yaw = transform->rotation.y + x_offset * controller->sensitivity;

        pitch = clamp(pitch, -89.0f, 89.0f);  // Clamp pitch to avoid flipping

        // Update last mouse values
        controller->mouse_x = curr_mouse_x;
        controller->mouse_y = curr_mouse_y;

        // Update target rotation
        transform->rotation.x = pitch;
        transform->rotation.y = yaw;
    }

    void update_controller(ECS &ecs, u32 id, const vec3 &front, const vec3 &right, const vec3 &up, f32 dt)
    {
        auto object = ecs.physics_objects[id].get();
        auto controller = ecs.camera_controllers[id].get();
        auto camera = ecs.cameras[id].get();
        auto transform = ecs.transforms[id].get();
        auto model = ecs.models[id].get();

        const vec3 BULLET_OFFSET = camera->target_offset;

        // Current state
        str player_state = PLAYER_STATE_IDLE;

        // Walk
        // -------------------------------------------------------------------------------------------------------------
        auto left_x = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_LEFT_X);
        auto left_y = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_LEFT_Y);

        if (fabs(left_x) >= TOLERANCE || fabs(left_y) >= TOLERANCE)
        {
            player_state = PLAYER_STATE_WALKING;
            if (left_y >= TOLERANCE) player_state = PLAYER_STATE_WALKING_BACK;

            vec3 clamped_front = vec3(front.x, 0.0f, front.z);
            if (fabs(left_y) >= TOLERANCE) object->force += clamped_front * controller->speed * -left_y * dt;
            if (fabs(left_x) >= TOLERANCE) object->force += right * controller->speed * left_x * dt;
        }

        // Turn
        // -------------------------------------------------------------------------------------------------------------
        auto right_x = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_RIGHT_X);
        auto right_y = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_RIGHT_Y);

        f32 x_offset = 0.0f;
        f32 y_offset = 0.0f;

        // Calculate X and Y offsets
        if (fabs(right_x) >= TOLERANCE) x_offset = right_x * 10.0f;

        if (fabs(right_y) >= TOLERANCE) y_offset = -right_y * 10.0f;

        // Calculate rotation
        f32 pitch = transform->rotation.x + y_offset * controller->sensitivity * dt;
        f32 yaw = transform->rotation.y + x_offset * controller->sensitivity * dt;

        pitch = clamp(pitch, MIN_PLAYER_PITCH, MAX_PLAYER_PITCH);  // Clamp pitch to avoid flipping

        // Update target rotation
        transform->rotation.x = pitch;
        transform->rotation.y = yaw;

        // Jump
        // -------------------------------------------------------------------------------------------------------------
        if (Input::is_joystick_button_pressed(JOYSTICK_2, GAMEPAD_BUTTON_CROSS))
        {
            if (player_timers[PLAYER_TIMER_STR_JUMP] <= PLAYER_TIMER_JUMP)
            {
                player_state = PLAYER_STATE_IDLE;  // @TODO: finish jumping state
                object->force += WORLD_UP * controller->speed.y * dt;
                player_timers[PLAYER_TIMER_STR_JUMP] += dt;
            }
        }

        // Reset jump
        else
        {
            if (player_timers[PLAYER_TIMER_STR_JUMP] > PLAYER_TIMER_JUMP)
                player_timers[PLAYER_TIMER_STR_JUMP_COOLDOWN] += dt;

            if (player_timers[PLAYER_TIMER_STR_JUMP_COOLDOWN] > PLAYER_TIMER_JUMP_COOLDOWN)
            {
                player_timers[PLAYER_TIMER_STR_JUMP_COOLDOWN] = 0.0f;
                player_timers[PLAYER_TIMER_STR_JUMP] = 0.0f;
            }
        }

        // Shoot
        // -------------------------------------------------------------------------------------------------------------
        auto trigger_left = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_LEFT_TRIGGER);
        auto trigger_right = Input::get_joystick_axis_value(JOYSTICK_2, GAMEPAD_AXIS_RIGHT_TRIGGER);

        // Normalize trigger value between [0, 2]
        trigger_left += 1.0f;
        trigger_right += 1.0f;

        // Zoom int
        if (trigger_left >= TOLERANCE) camera->target_zoom = MIN_CAMERA_ZOOM;

        // Zoom out
        else
            camera->target_zoom = MAX_CAMERA_ZOOM;

        camera->target_zoom = clamp(camera->target_zoom, MIN_CAMERA_ZOOM, MAX_CAMERA_ZOOM);

        // Shoot
        if (trigger_right >= TOLERANCE)
        {
            if (player_timers[PLAYER_TIMER_STR_SHOOT_COOLDOWN] >= PLAYER_TIMER_SHOOT_COOLDOWN)
            {
                player_state = PLAYER_STATE_SHOOTING;
                player_shooting = true;

                Transform bullet_transform = *transform;
                bullet_transform.position = bullet_transform.position + right * BULLET_OFFSET.x;
                bullet_transform.position = bullet_transform.position + up * BULLET_OFFSET.y;
                bullet_transform.position = bullet_transform.position + front * BULLET_OFFSET.z;

                bullet_transform.scale = vec3(15.0f);

                PhysicsObject bullet_object = PhysicsObject(vec3(0.0f), vec3(100000.0f), front * 1'000'000.0f, 15.0f);

                shoot(ecs, bullet_transform, bullet_object);
                player_timers[PLAYER_TIMER_STR_SHOOT_COOLDOWN] = 0.0f;
            }
        }

        player_timers[PLAYER_TIMER_STR_SHOOT_COOLDOWN] =
            clamp(player_timers[PLAYER_TIMER_STR_SHOOT_COOLDOWN] + dt, 0.0f, PLAYER_TIMER_SHOOT_COOLDOWN);

        // Wait for shooting animation to finish
        if (player_shooting)
        {
            auto animation_dur = model->model->animations[PLAYER_STATE_SHOOTING]->get_duration_seconds();

            player_timers[PLAYER_TIMER_STR_SHOOT_ANIMATION] =
                clamp(player_timers[PLAYER_TIMER_STR_SHOOT_ANIMATION] + dt, 0.0f, animation_dur);

            // Animation is finished, proceed to next state
            if (player_timers[PLAYER_TIMER_STR_SHOOT_ANIMATION] >= animation_dur)
            {
                player_timers[PLAYER_TIMER_STR_SHOOT_ANIMATION] = 0.0f;
                player_shooting = false;
            }

            // Animation is still going, stay in the shooting state
            else
                player_state = PLAYER_STATE_SHOOTING;
        }

        update_state_machine(ecs, id, player_state, dt);
    }

    void shoot(ECS &ecs, const Transform &transform, const PhysicsObject &object)
    {
        auto &audio_engine = Game::get().get_audio_engine();
        bullet(ecs, transform, object, 0, 5.0f, 25.0f, 1.0f);

        audio_engine.load("player_shoot_sfx", "bloss1/assets/sounds/709042__funnyvoices__shotgun-eu-fx.wav");
        audio_engine.play("player_shoot_sfx");
    }
};  // namespace bls
