#include "camera/camera.hpp"

namespace bls
{
    Camera::Camera(vec3 position, vec3 world_up, vec3 world_front,
                   f32 pitch, f32 yaw, f32 roll,
                   f32 zoom, f32 near, f32 far)
    {
        this->position = position;
        this->world_up = world_up;
        this->world_front = world_front;

        this->yaw = yaw;
        this->pitch = pitch;
        this->roll = roll;

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

    void Camera::set_rotation(f32 pitch, f32 yaw, f32 roll)
    {
        this->pitch = pitch;
        this->yaw = yaw;
        this->roll = roll;
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
        return vec3(pitch, yaw, roll);
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
        // View matrix is the inverse of the camera's model matrix
        mat4 rotate_x = rotate(mat4(1.0f), radians(pitch), vec3(1.0f, 0.0f, 0.0f));
        mat4 rotate_y = rotate(mat4(1.0f), radians(yaw),   vec3(0.0f, 1.0f, 0.0f));
        mat4 rotate_z = rotate(mat4(1.0f), radians(roll),  vec3(0.0f, 0.0f, 1.0f));

        mat4 translate = glm::translate(mat4(1.0f), position);

        mat4 model = translate * rotate_z * rotate_y * rotate_x;

        view_matrix = inverse(model);

        // Update direction vectors
        mat3 rotation = mat3(model);

        front = normalize(rotation * world_front);
        right = normalize(cross(front, world_up));
        up    = normalize(cross(right, front));
    }
};
