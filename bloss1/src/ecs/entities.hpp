#pragma once

/**
 * @brief The entities of the ECS. Might wanna split in other files if it gets too big.
 */

#include "core/game.hpp"
#include "ecs/ecs.hpp"
#include "renderer/buffers.hpp"

namespace bls
{
    u32 player(ECS& ecs, const Transform& transform)
    {
        u32 id = ecs.get_id();

        // auto model = Model::create("player", "bloss1/assets/models/box/planter_box_01_2k.gltf", false);
        auto model = Model::create("player", "bloss1/assets/models/sphere/rusted_sphere.gltf", false);

        ecs.models[id] = std::make_unique<ModelComponent>(model.get());
        ecs.transforms[id] = std::make_unique<Transform>(transform);
        ecs.physics_objects[id] = std::make_unique<PhysicsObject>();
        ecs.colliders[id] = std::make_unique<SphereCollider>(10.0f);

        return id;
    }

    u32 floor(ECS& ecs, const Transform& transform)
    {
        u32 id = ecs.get_id();

        auto model = Model::create("sponza", "bloss1/assets/models/floor/square_floor_4k.gltf", false);

        ecs.models[id] = std::make_unique<ModelComponent>(model.get());
        ecs.transforms[id] = std::make_unique<Transform>(transform);
        ecs.colliders[id] = std::make_unique<PlaneCollider>(transform.position, transform.scale.x);

        return id;
    }

    u32 directional_light(ECS& ecs, const Transform& transform, const DirectionalLight& light)
    {
        u32 id = ecs.get_id();

        // @TODO: dont forget to create a debugging model (and shader for that model)
        // auto model = Model::create("directional_light", "bloss1/assets/models/debug", false);

        // ecs.models[id] = std::make_unique<ModelComponent>(model.get());
        ecs.dir_lights[id] = std::make_unique<DirectionalLight>(light);
        ecs.transforms[id] = std::make_unique<Transform>(transform);

        return id;
    }

    u32 point_light(ECS& ecs, const Transform& transform, const PointLight& light)
    {
        u32 id = ecs.get_id();

        // @TODO: dont forget to create a debugging model (and shader for that model)
        // auto model = Model::create("directional_light", "bloss1/assets/models/debug", false);

        // ecs.models[id] = std::make_unique<ModelComponent>(model.get());
        ecs.point_lights[id] = std::make_unique<PointLight>(light);
        ecs.transforms[id] = std::make_unique<Transform>(transform);

        return id;
    }
};
