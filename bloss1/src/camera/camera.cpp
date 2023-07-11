#include "camera/camera.hpp"

namespace bls
{
    Camera::Camera(vec3 target_position, vec3 target_offset,
                   f32 target_pitch, f32 target_yaw,
                   vec3 world_up,
                   f32 zoom, f32 near, f32 far,
                   f32 lerp_factor)
    {
        this->target_position = target_position;
        this->target_offset = target_offset;
        this->position = target_position;

        this->target_pitch = target_pitch;
        this->target_yaw = target_yaw;
        this->target_zoom = zoom;

        this->world_up = world_up;

        this->zoom = zoom;
        this->near = near;
        this->far = far;

        // Higher is more responsive
        this->lerp_factor = lerp_factor;

        front = vec3(cos(radians(target_yaw)) * cos(radians(target_pitch)),
                     sin(radians(target_pitch)),
                     sin(radians(target_yaw)) * cos(radians(target_pitch)));
        front = normalize(front);

        right = normalize(cross(front, world_up));
        up    = normalize(cross(right, front));
    }

    Camera::~Camera()
    {
        std::cout << "camera destroyed successfully\n";
    }

    void Camera::set_target_position(const vec3& position)
    {
        target_position = position;
    }

    void Camera::set_target_rotation(f32 pitch, f32 yaw)
    {
        target_pitch = pitch;
        target_yaw = yaw;
    }

    void Camera::set_target_offset(const vec3& offset)
    {
        target_offset = offset;
    }

    void Camera::set_target_zoom(f32 zoom)
    {
        this->target_zoom = zoom;
    }

    mat4 Camera::get_view_matrix()
    {
        return view_matrix;
    }

    mat4 Camera::get_projection_matrix(f32 width, f32 height)
    {
        // Projection matrix (Perspective)
        return perspective(radians(zoom), width / height, near, far);

        // Projection matrix (Ortho)
        // return ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 100.0f); // In world coordinates
    }

    vec3 Camera::get_position()
    {
        return position;
    }

    f32 Camera::get_zoom()
    {
        return zoom;
    }

    f32 Camera::get_near()
    {
        return near;
    }

    f32 Camera::get_far()
    {
        return far;
    }

    void Camera::update(f32 dt)
    {
        auto target_front = vec3(cos(radians(target_yaw)) * cos(radians(target_pitch)),
                                 sin(radians(target_pitch)),
                                 sin(radians(target_yaw)) * cos(radians(target_pitch)));
        target_front = normalize(target_front);

        auto target_right = normalize(cross(target_front, world_up));
        auto target_up    = normalize(cross(target_right, target_front));

        target_position = target_position + target_front * (-target_offset.z);
        target_position = target_position + target_up * target_offset.y;
        target_position = target_position + target_right * target_offset.x;

        f32 delta_lerp = clamp(lerp_factor * dt, 0.0f, 1.0f);
        position = mix(position, target_position, delta_lerp);
        front    = mix(front, target_front, delta_lerp);
        up       = mix(up, target_up, delta_lerp);

        zoom = mix(zoom, target_zoom, delta_lerp);

        view_matrix = look_at(position, position + front, up);
    }
};
