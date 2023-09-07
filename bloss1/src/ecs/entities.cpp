#include "ecs/entities.hpp"
#include "core/game.hpp"
#include "renderer/model.hpp"

namespace bls
{
    u32 player(ECS& ecs, const Transform& transform)
    {
        u32 id = ecs.get_id();

        auto model = Model::create("player", "bloss1/assets/models/sphere/rusted_sphere.gltf", false);

        ecs.names[id] = "player";
        ecs.models[id] = std::make_unique<ModelComponent>(model.get());
        ecs.transforms[id] = std::make_unique<Transform>(transform);
        ecs.physics_objects[id] = std::make_unique<PhysicsObject>();
        ecs.colliders[id] = std::make_unique<SphereCollider>(transform.scale.x);
        // ecs.colliders[id] = std::make_unique<BoxCollider>(transform.scale.x * 0.95f, transform.scale.y * 0.95f, transform.scale.z * 0.95f);

        ecs.cameras[id] = std::make_unique<Camera>(vec3(15.0f, 7.0f, 50.0f));
        ecs.camera_controllers[id] = std::make_unique<CameraController>();

        return id;
    }

    u32 ball(ECS& ecs, const Transform& transform)
    {
        u32 id = ecs.get_id();

        auto model = Model::create("ball", "bloss1/assets/models/sphere/rusted_sphere.gltf", false);

        ecs.names[id] = "ball";
        ecs.models[id] = std::make_unique<ModelComponent>(model.get());
        ecs.transforms[id] = std::make_unique<Transform>(transform);
        ecs.physics_objects[id] = std::make_unique<PhysicsObject>();
        ecs.colliders[id] = std::make_unique<SphereCollider>(transform.scale.x);

        return id;
    }

    u32 vampire(ECS& ecs, const Transform& transform)
    {
        u32 id = ecs.get_id();

        auto model = Model::create("vampire", "bloss1/assets/models/vampire/dancing_vampire.dae", true);

        ecs.names[id] = "vampire";
        ecs.models[id] = std::make_unique<ModelComponent>(model.get());
        ecs.transforms[id] = std::make_unique<Transform>(transform);
        ecs.physics_objects[id] = std::make_unique<PhysicsObject>();
        ecs.colliders[id] = std::make_unique<BoxCollider>(
                                5.0f, 5.0f, 5.0f,
                                vec3(0.0f, 5.0f, 0.0f));

        return id;
    }

    u32 abomination(ECS& ecs, const Transform& transform)
    {
        u32 id = ecs.get_id();

        auto model = Model::create("abomination", "bloss1/assets/models/abomination/abomination.fbx", false);

        ecs.names[id] = "abomination";
        ecs.models[id] = std::make_unique<ModelComponent>(model.get());
        ecs.transforms[id] = std::make_unique<Transform>(transform);
        ecs.colliders[id] = std::make_unique<BoxCollider>(3.0f, 3.0f, 3.0f, vec3(0.0f), true);
        ecs.timers[id] = std::make_unique<Timer>();

        std::vector<KeyFrame> key_frames;
        key_frames.push_back({ transform, 3.0f });
        key_frames.push_back({ Transform(transform.position + vec3(0.0f, 20.0f, 0.0f), vec3(0.0f, 90.0f, 0.0f), transform.scale / 2.0f), 2.0f });
        key_frames.push_back({ Transform(transform.position - vec3(0.0f, 20.0f, 0.0f), vec3(0.0f), transform.scale), 1.0f });
        ecs.transform_animations[id] = std::make_unique<TransformAnimation>(key_frames);

        return id;
    }

    u32 floor(ECS& ecs, const Transform& transform)
    {
        u32 id = ecs.get_id();

        auto model = Model::create("floor", "bloss1/assets/models/floor/square_floor_fixed.gltf", false);

        ecs.names[id] = "floor";
        ecs.models[id] = std::make_unique<ModelComponent>(model.get());
        ecs.transforms[id] = std::make_unique<Transform>(transform);
        ecs.physics_objects[id] = std::make_unique<PhysicsObject>();
        ecs.colliders[id] = std::make_unique<BoxCollider>(
                                transform.scale.x * 10.0f, transform.scale.y * 20.0f, transform.scale.z * 10.0f,
                                vec3(0.0f, -transform.scale.y * 20.0f, 0.0f),
                                true);

        return id;
    }

    u32 directional_light(ECS& ecs, const Transform& transform, const DirectionalLight& light)
    {
        u32 id = ecs.get_id();

        // @TODO: dont forget to create a debugging model (and shader for that model)
        // auto model = Model::create("directional_light", "bloss1/assets/models/debug", false);

        // ecs.models[id] = std::make_unique<ModelComponent>(model.get());
        ecs.names[id] = "directional_light";
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
        ecs.names[id] = "point_light";
        ecs.point_lights[id] = std::make_unique<PointLight>(light);
        ecs.transforms[id] = std::make_unique<Transform>(transform);

        return id;
    }
};