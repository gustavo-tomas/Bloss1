#pragma once

/**
 * @brief The components of the ECS. Might wanna split in other files if it gets too big.
 */

#include "ecs/ecs.hpp"
#include "math/math.hpp"

namespace bls
{
    // Component: contain the data
    class Component
    {
        public:
            virtual ~Component() { }
    };

    class Transform : public Component
    {
        public:
            Transform(const vec3& position = vec3(0.0f), const vec3& rotation = vec3(0.0f), const vec3& scale = vec3(1.0f))
                : position(position), rotation(rotation), scale(scale) { }

            vec3 position;
            vec3 rotation;
            vec3 scale;
    };

    class Camera : public Component
    {
        public:
            Camera(vec3 target_offset = vec3(0.0f),
                   vec3 world_up = vec3(0.0f, 1.0f, 0.0f),
                   f32 zoom = 45.0f, f32 near = 1.0f, f32 far = 1000.0f,
                   f32 lerp_factor = 7.5f)

                : target_offset(target_offset),
                  world_up(world_up),
                  zoom(zoom), near(near), far(far), lerp_factor(lerp_factor),
                  target_zoom(zoom),
                  position(vec3(0.0f)) { }

            mat4 view_matrix, projection_matrix;
            vec3 target_offset, world_up;
            f32 zoom, near, far, lerp_factor, target_zoom;
            vec3 position, front, right, up;
    };

    class CameraController : public Component
    {
        public:
            CameraController(f32 speed = 80.0f, f32 sensitivity = 0.03f)
                : speed(speed), sensitivity(sensitivity), mouse_x(0.0f), mouse_y(0.0f) { }

            f32 speed, sensitivity;
            f32 mouse_x, mouse_y;
    };

    class DirectionalLight : public Component
    {
        public:
            DirectionalLight(const vec3& ambient = vec3(0.0f), const vec3& diffuse = vec3(1.0f), const vec3& specular = vec3(1.0f))
                : ambient(ambient), diffuse(diffuse), specular(specular) { }

            vec3 ambient;
            vec3 diffuse;
            vec3 specular;
    };

    class PointLight : public Component
    {
        public:
            PointLight(const vec3& ambient = vec3(0.0f), const vec3& diffuse = vec3(1.0f), const vec3& specular = vec3(1.0f),
                       f32 constant = 1.0f, f32 linear = 0.0001f, f32 quadratic = 0.000001f)
                : ambient(ambient), diffuse(diffuse), specular(specular),
                  constant(constant), linear(linear), quadratic(quadratic) { }

            vec3 ambient;
            vec3 diffuse;
            vec3 specular;

            f32 constant;
            f32 linear;
            f32 quadratic;
    };

    // @TODO: find a better solution (get rid of model entirely?)
    class Model;
    class ModelComponent : public Component
    {
        public:
            ModelComponent(Model* model)
                : model(model) { }

            Model* model;
    };

    class PhysicsObject : public Component
    {
        public:
            PhysicsObject(const vec3& velocity = vec3(0.0f), const vec3& force = vec3(0.0f), f32 mass = 1.0f)
                : velocity(velocity), force(force), mass(mass) { }

            vec3 velocity;
            vec3 force;
            f32 mass;
    };

    // Collider interface
    class Collider : public Component
    {
        public:
            enum ColliderType
            {
                Box, Sphere
            };

            static str get_collider_str(ColliderType type)
            {
                switch (type)
                {
                    case Sphere:
                        return "sphere";
                        break;

                    case Box:
                        return "box";
                        break;

                    default:
                        std::cerr << "invalid collider type\n";
                        return "";
                        break;
                }
            }

            Collider(ColliderType type, const vec3& offset, bool immovable)
                : type(type), offset(offset), immovable(immovable) { }

            virtual ~Collider() { }

            ColliderType type;
            vec3 offset; // Offset to the component position
            vec3 color;
            bool immovable;
    };

    class BoxCollider : public Collider
    {
        public:
            BoxCollider(const vec3& dimensions, const vec3& offset = vec3(0.0f), bool immovable = false)
                : Collider(ColliderType::Box, offset, immovable), dimensions(dimensions) { }

            vec3 dimensions; // width, height, depth
    };

    class SphereCollider : public Collider
    {
        public:
            SphereCollider(f32 radius, const vec3& offset = vec3(0.0f), bool immovable = false)
                : Collider(ColliderType::Sphere, offset, immovable), radius(radius) { }

            f32 radius;
    };

    class Timer : public Component
    {
        public:
            Timer()
                : time(0.0f) { }

            f32 time;
    };

    class Sound : public Component
    {
        public:
            Sound(const str& name, f32 volume, bool play_now)
                : name(name), volume(volume), play_now(play_now) { }

            str name;
            f32 volume;
            bool play_now;
    };

    // @TODO: same problem as model
    class Font;
    class Text : public Component
    {
        public:
            Text(Font* font, const str& font_file, const str& text, const vec3& color)
                : font(font), font_file(font_file), text(text), color(color) { }

            Font* font;
            str font_file;
            str text;
            vec3 color;
    };

    // Key frame for animating transforms
    struct KeyFrame
    {
        Transform transform;
        f32 duration;
    };

    class TransformAnimation : public Component
    {
        public:
            TransformAnimation(std::vector<KeyFrame> key_frames)
                : key_frames(key_frames), curr_frame_idx(0) { }

            std::vector<KeyFrame> key_frames;
            u32 curr_frame_idx;
    };
};
