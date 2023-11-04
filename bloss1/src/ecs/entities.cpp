#include "ecs/entities.hpp"

#include "core/game.hpp"
#include "renderer/font.hpp"
#include "renderer/model.hpp"

namespace bls
{
    u32 empty_entity(ECS &ecs)
    {
        return ecs.get_id();
    }

    u32 player(ECS &ecs, const Transform &transform)
    {
        u32 id = ecs.get_id();

        auto model = Model::create("player", "bloss1/assets/models/sphere/rusted_sphere.gltf", false);

        ecs.names[id] = "player";
        ecs.models[id] = std::make_unique<ModelComponent>(model.get());
        ecs.transforms[id] = std::make_unique<Transform>(transform);
        ecs.physics_objects[id] = std::make_unique<PhysicsObject>();
        ecs.colliders[id] = std::make_unique<SphereCollider>(transform.scale.x);
        // ecs.colliders[id] = std::make_unique<BoxCollider>(transform.scale.x * 0.95f, transform.scale.y * 0.95f,
        // transform.scale.z * 0.95f);

        ecs.cameras[id] = std::make_unique<Camera>(vec3(15.0f, 7.0f, 50.0f));
        ecs.camera_controllers[id] = std::make_unique<CameraController>();
        // ecs.sounds[id]["player_fire"] = std::make_unique<Sound>("player_fire", 0.5f, false);

        // auto& audio_engine = Game::get().get_audio_engine();
        // audio_engine.load("player_fire", "bloss1/assets/sounds/gunshot.mp3");

        return id;
    }

    u32 bullet(ECS &ecs,
               const Transform &transform,
               const PhysicsObject &object,
               u32 sender_id,
               f32 damage,
               f32 explosion_radius,
               f32 explosion_duration)
    {
        auto id = ecs.get_id();
        auto model = Model::create("bullet", "bloss1/assets/models/bullet/bullet.fbx", false);

        ecs.names[id] = "bullet";
        ecs.models[id] = std::make_unique<ModelComponent>(model.get());
        ecs.transforms[id] = std::make_unique<Transform>(transform);
        ecs.physics_objects[id] = std::make_unique<PhysicsObject>(object);
        ecs.colliders[id] =
            std::make_unique<SphereCollider>(transform.scale.x / 5.0f,
                                             vec3(0.0f),
                                             false,
                                             Collider::ColliderMask::Projectile,  // description
                                             Collider::ColliderMask::World |      // interaction
                                                 Collider::ColliderMask::Player | Collider::ColliderMask::Enemy);
        ecs.projectiles[id] = std::make_unique<Projectile>(sender_id, damage, explosion_radius, explosion_duration);
        ecs.timers[id] = std::make_unique<Timer>();

        auto *emitter = new SphereEmitter(transform.position, false, transform.scale.x / 6.25f);
        // auto *emitter = new BoxEmitter(transform.position, false, vec3(transform.scale.x / 5.0f));
        // auto *emitter = new PointEmitter(transform.position, false);

        auto particle = emitter->get_particle();

        // Player bullet
        if (sender_id == 0)
        {
            particle.color_begin = vec4(0.0f, 0.8f, 0.81f, 1.0f);
            particle.color_end = vec4(0.68f, 0.93f, 0.93f, 1.0f);
            particle.life_time = 0.75f;
        }

        // Enemy bullet
        else
        {
            particle.color_begin = vec4(0.95f, 0.0f, 0.0f, 1.0f);
            particle.color_end = vec4(0.95f, 0.93f, 0.0f, 1.0f);
            particle.life_time = 0.75f;
        }
        emitter->set_particle(particle);

        ecs.particle_systems[id] = std::make_unique<ParticleSystem>(emitter, 40, 0.01f);

        return id;
    }

    u32 ball(ECS &ecs, const Transform &transform)
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

    u32 vampire(ECS &ecs, const Transform &transform)
    {
        u32 id = ecs.get_id();

        auto model = Model::create("vampire", "bloss1/assets/models/vampire/dancing_vampire.dae", false);

        ecs.names[id] = "vampire";
        ecs.models[id] = std::make_unique<ModelComponent>(model.get());
        ecs.transforms[id] = std::make_unique<Transform>(transform);
        ecs.physics_objects[id] = std::make_unique<PhysicsObject>();
        ecs.colliders[id] = std::make_unique<BoxCollider>(vec3(5.0f, 5.0f, 5.0f), vec3(0.0f, 5.0f, 0.0f));

        return id;
    }

    u32 abomination(ECS &ecs, const Transform &transform)
    {
        u32 id = ecs.get_id();

        auto model = Model::create("abomination", "bloss1/assets/models/abomination/abomination.fbx", false);

        ecs.names[id] = "abomination";
        ecs.models[id] = std::make_unique<ModelComponent>(model.get());
        ecs.transforms[id] = std::make_unique<Transform>(transform);
        ecs.colliders[id] = std::make_unique<BoxCollider>(vec3(3.0f, 3.0f, 3.0f), vec3(0.0f), true);
        ecs.timers[id] = std::make_unique<Timer>();

        std::vector<KeyFrame> key_frames;
        key_frames.push_back({transform, 3.0f});
        key_frames.push_back(
            {Transform(transform.position + vec3(0.0f, 20.0f, 0.0f), vec3(0.0f, 90.0f, 0.0f), transform.scale / 2.0f),
             2.0f});
        key_frames.push_back(
            {Transform(transform.position - vec3(0.0f, 20.0f, 0.0f), vec3(0.0f), transform.scale), 1.0f});
        ecs.transform_animations[id] = std::make_unique<TransformAnimation>(key_frames);

        return id;
    }

    u32 floor(ECS &ecs, const Transform &transform)
    {
        u32 id = ecs.get_id();

        auto model = Model::create("floor", "bloss1/assets/models/floor/square_floor_fixed.gltf", false);

        ecs.names[id] = "floor";
        ecs.models[id] = std::make_unique<ModelComponent>(model.get());
        ecs.transforms[id] = std::make_unique<Transform>(transform);
        ecs.physics_objects[id] = std::make_unique<PhysicsObject>();
        ecs.colliders[id] = std::make_unique<BoxCollider>(
            vec3(transform.scale.x * 10.0f, transform.scale.y * 20.0f, transform.scale.z * 10.0f),
            vec3(0.0f, -transform.scale.y * 20.0f, 0.0f),
            true);

        return id;
    }

    u32 directional_light(ECS &ecs, const Transform &transform, const DirectionalLight &light)
    {
        u32 id = ecs.get_id();

        ecs.names[id] = "directional_light";
        ecs.dir_lights[id] = std::make_unique<DirectionalLight>(light);
        ecs.transforms[id] = std::make_unique<Transform>(transform);

        return id;
    }

    u32 point_light(ECS &ecs, const Transform &transform, const PointLight &light)
    {
        u32 id = ecs.get_id();

        ecs.names[id] = "point_light";
        ecs.point_lights[id] = std::make_unique<PointLight>(light);
        ecs.transforms[id] = std::make_unique<Transform>(transform);

        return id;
    }

    u32 text(ECS &ecs, const Transform &transform, const str &text, const vec3 &color)
    {
        u32 id = ecs.get_id();

        auto font = Font::create("inder", "bloss1/assets/fonts/inder_regular.ttf");

        ecs.names[id] = "text";
        ecs.transforms[id] = std::make_unique<Transform>(transform);
        ecs.texts[id] = std::make_unique<Text>(font.get(), "bloss1/assets/fonts/inder_regular.ttf", text, color);

        return id;
    }

    u32 background_music(ECS &ecs, const Transform &transform, const Sound &sound, const str &file, bool looping)
    {
        u32 id = ecs.get_id();

        auto &audio_engine = Game::get().get_audio_engine();
        audio_engine.load(sound.name, file, looping);

        ecs.names[id] = "background_music";
        ecs.transforms[id] = std::make_unique<Transform>(transform);
        ecs.sounds[id][sound.name] = std::make_unique<Sound>(sound);

        return id;
    }
};  // namespace bls
