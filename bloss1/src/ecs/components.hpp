#pragma once

/**
 * @brief The components of the ECS. Might wanna split in other files if it gets too big.
 */

#include "ecs/ecs.hpp"
#include "math/math.hpp"
#include "renderer/buffers.hpp"
#include "renderer/model.hpp"

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
    class ModelComponent : public Component
    {
        public:
            ModelComponent(Model* model)
                : model(model) { }

            Model* model;
    };
};
