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

    class Light : public Component
    {
        public:
            Light(const vec3& ambient = vec3(0.0f), const vec3& diffuse = vec3(1.0f), const vec3& specular = vec3(1.0f))
                : ambient(ambient), diffuse(diffuse), specular(specular) { }

            vec3 ambient;
            vec3 diffuse;
            vec3 specular;
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
