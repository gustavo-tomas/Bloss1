#include "camera/camera.hpp"

namespace bls
{
    Camera::Camera(vec3 position, vec3 front, vec3 world_up,
                   f32 yaw, f32 pitch,
                   f32 zoom, f32 near, f32 far)
    {
        this->position = position;

        this->front = front;
        this->world_up = world_up;

        this->yaw = yaw;
        this->pitch = pitch;

        this->zoom = zoom;

        this->near = near;
        this->far = far;

        update_view_matrix();
    }

    Camera::~Camera()
    {
        std::cout << "camera destroyed successfully\n";
    }

    void Camera::set_position(const vec3& position)
    {
        this->position = position;
        update_view_matrix();
    }

    void Camera::set_rotation(f32 pitch, f32 yaw)
    {
        this->yaw = yaw;
        this->pitch = pitch;
        update_view_matrix();
    }

    void Camera::set_zoom(f32 zoom)
    {
        this->zoom = zoom;
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

    vec3 Camera::get_rotation()
    {
        return vec3(pitch, yaw, 0.0f);
    }

    vec3 Camera::get_front()
    {
        return front;
    }

    vec3 Camera::get_right()
    {
        return right;
    }

    vec3 Camera::get_up()
    {
        return up;
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

    void Camera::update_view_matrix()
    {
        front = vec3(cos(radians(yaw)) * cos(radians(pitch)),
                     sin(radians(pitch)),
                     sin(radians(yaw)) * cos(radians(pitch)));
        front = normalize(front);

        right = normalize(cross(front, world_up));
        up    = normalize(cross(right, front));

        view_matrix = look_at(position, position + front, up);
    }
};
