#pragma once

/**
 * @brief The components of the ECS. Might wanna split in other files if it gets too big.
 */

#include "ecs/ecs.hpp"

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
            Transform(const i32& position = 0, const i32& rotation = 0, const i32& scale = 0)
                : position(position), rotation(rotation), scale(scale) { }

            i32 position;
            i32 rotation;
            i32 scale;
    };

    class Model : public Component
    {
        public:
            Model(const i32& mesh = 0)
                : mesh(mesh) { }

            i32 mesh;
    };
};
