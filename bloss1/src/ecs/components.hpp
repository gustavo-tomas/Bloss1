#pragma once

/**
 * @brief The components of the ECS. Might wanna split in other files if it gets too big.
 */

#include "ecs/ecs.hpp"
#include "math/math.hpp"
#include "core/timer.hpp"

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
            PointLight(const vec3& diffuse = vec3(1.0f), const vec3& ambient = vec3(0.0f), const vec3& specular = vec3(1.0f),
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

    // @TODO: for now, the object position is the same as the model's
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

            Collider(ColliderType type, bool immovable)
                : type(type), immovable(immovable) { }

            virtual ~Collider() { }

            ColliderType type;
            vec3 color;
            bool immovable;
    };

    class BoxCollider : public Collider
    {
        public:
            BoxCollider(f32 width, f32 height, f32 depth, bool immovable = false)
                : Collider(ColliderType::Box, immovable), width(width), height(height), depth(depth) { }

            f32 width, height, depth;
    };

    class SphereCollider : public Collider
    {
        public:
            SphereCollider(f32 radius, bool immovable = false)
                : Collider(ColliderType::Sphere, immovable), radius(radius) { }

            f32 radius;
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
            Timer timer;
    };
};
