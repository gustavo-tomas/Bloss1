#pragma once

/**
 * @brief The components of the ECS. Might wanna split in other files if it gets too big.
 */

#include "core/logger.hpp"
#include "ecs/ecs.hpp"
#include "ecs/state_machine.hpp"
#include "ecs/systems/particle_system.hpp"
#include "math/math.hpp"

namespace bls
{
    // Component: contain the data
    class Component
    {
        public:
            virtual ~Component()
            {
            }
    };

    class Transform : public Component
    {
        public:
            Transform(const vec3 &position = vec3(0.0f),
                      const vec3 &rotation = vec3(0.0f),
                      const vec3 &scale = vec3(1.0f))
                : position(position), rotation(rotation), scale(scale)
            {
            }

            vec3 position;
            vec3 rotation;
            vec3 scale;
    };

    class Camera : public Component
    {
        public:
            Camera(vec3 target_offset = vec3(0.0f),
                   vec3 world_up = vec3(0.0f, 1.0f, 0.0f),
                   f32 zoom = 45.0f,
                   f32 near = 1.0f,
                   f32 far = 1000.0f,
                   f32 lerp_factor = 7.5f)

                : target_offset(target_offset),
                  world_up(world_up),
                  zoom(zoom),
                  near(near),
                  far(far),
                  lerp_factor(lerp_factor),
                  target_zoom(zoom),
                  position(vec3(0.0f))
            {
            }

            mat4 view_matrix, projection_matrix;
            vec3 target_offset, world_up;
            f32 zoom, near, far, lerp_factor, target_zoom;
            vec3 position, front, right, up;
    };

    class CameraController : public Component
    {
        public:
            CameraController(const vec3 &speed = vec3(100.0f), f32 sensitivity = 0.03f)
                : speed(speed), sensitivity(sensitivity), mouse_x(0.0f), mouse_y(0.0f)
            {
            }

            vec3 speed;
            f32 sensitivity;
            f32 mouse_x, mouse_y;
    };

    class DirectionalLight : public Component
    {
        public:
            DirectionalLight(const vec3 &ambient = vec3(0.0f),
                             const vec3 &diffuse = vec3(1.0f),
                             const vec3 &specular = vec3(1.0f))
                : ambient(ambient), diffuse(diffuse), specular(specular)
            {
            }

            vec3 ambient;
            vec3 diffuse;
            vec3 specular;
    };

    class PointLight : public Component
    {
        public:
            PointLight(const vec3 &ambient = vec3(0.0f),
                       const vec3 &diffuse = vec3(1.0f),
                       const vec3 &specular = vec3(1.0f),
                       f32 constant = 1.0f,
                       f32 linear = 0.0001f,
                       f32 quadratic = 0.000001f)
                : ambient(ambient),
                  diffuse(diffuse),
                  specular(specular),
                  constant(constant),
                  linear(linear),
                  quadratic(quadratic)
            {
            }

            vec3 ambient;
            vec3 diffuse;
            vec3 specular;

            f32 constant;
            f32 linear;
            f32 quadratic;
    };

    class Model;
    class ModelComponent : public Component
    {
        public:
            ModelComponent(Model *model) : model(model)
            {
            }

            Model *model;
    };

    class PhysicsObject : public Component
    {
        public:
            PhysicsObject(const vec3 &velocity = vec3(0.0f),
                          const vec3 &terminal_velocity = vec3(100.0f),
                          const vec3 &force = vec3(0.0f),
                          f32 mass = 1.0f)
                : velocity(velocity), terminal_velocity(terminal_velocity), force(force), mass(mass)
            {
            }

            vec3 velocity;
            vec3 terminal_velocity;
            vec3 force;
            f32 mass;
    };

    // Collider interface
    class Collider : public Component
    {
        public:
            enum class ColliderType
            {
                Box,
                Sphere
            };

            enum ColliderMask
            {
                World = 0x01,
                Player = 0x02,
                Enemy = 0x04,
                Projectile = 0x08
            };

            static str get_collider_str(ColliderType type)
            {
                switch (type)
                {
                    case ColliderType::Sphere:
                        return "sphere";
                        break;

                    case ColliderType::Box:
                        return "box";
                        break;

                    default:
                        LOG_ERROR("invalid collider type");
                        return "";
                        break;
                }
            }

            Collider(ColliderType type, const vec3 &offset, bool immovable, u32 description_mask, u32 interaction_mask)
                : type(type),
                  offset(offset),
                  immovable(immovable),
                  description_mask(description_mask),
                  interaction_mask(interaction_mask)
            {
            }

            virtual ~Collider()
            {
            }

            ColliderType type;
            vec3 offset;  // Offset to the component position
            vec3 color;
            bool immovable;
            u32 description_mask, interaction_mask;
    };

    class BoxCollider : public Collider
    {
        public:
            BoxCollider(const vec3 &dimensions,
                        const vec3 &offset = vec3(0.0f),
                        bool immovable = false,
                        u32 description_mask = ColliderMask::World,
                        u32 interaction_mask = ColliderMask::World)
                : Collider(ColliderType::Box, offset, immovable, description_mask, interaction_mask),
                  dimensions(dimensions)
            {
            }

            vec3 dimensions;  // width, height, depth
    };

    class SphereCollider : public Collider
    {
        public:
            SphereCollider(f32 radius,
                           const vec3 &offset = vec3(0.0f),
                           bool immovable = false,
                           u32 description_mask = ColliderMask::World,
                           u32 interaction_mask = ColliderMask::World)
                : Collider(ColliderType::Sphere, offset, immovable, description_mask, interaction_mask), radius(radius)
            {
            }

            f32 radius;
    };

    class Timer : public Component
    {
        public:
            Timer(f32 time = 0.0f) : time(time)
            {
            }

            f32 time;
    };

    class Sound : public Component
    {
        public:
            Sound(const str &file, const str &name, f32 volume, bool play_now, bool looping)
                : file(file), name(name), volume(volume), play_now(play_now), looping(looping)
            {
            }

            str file;
            str name;
            f32 volume;
            bool play_now;
            bool looping;
    };

    class Font;
    class Text : public Component
    {
        public:
            Text(Font *font, const str &font_file, const str &text, const vec3 &color)
                : font(font), font_file(font_file), text(text), color(color)
            {
            }

            Font *font;
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
            TransformAnimation(std::vector<KeyFrame> key_frames) : key_frames(key_frames), curr_frame_idx(0)
            {
            }

            std::vector<KeyFrame> key_frames;
            u32 curr_frame_idx;
    };

    class State;
    class StateMachine : public Component
    {
        public:
            StateMachine(const str &current_state) : current_state(current_state)
            {
                state = std::make_unique<State>();
            }

            std::unique_ptr<State> state;
            str current_state;
    };

    class Projectile : public Component
    {
        public:
            Projectile(u32 sender_id,
                       f32 damage = 5.0f,
                       f32 explosion_radius = 10.0f,
                       f32 explosion_duration = 1.0f,
                       f32 time_to_live = 5.0f)
                : sender_id(sender_id),
                  damage(damage),
                  explosion_radius(explosion_radius),
                  explosion_duration(explosion_duration),
                  time_to_live(time_to_live)
            {
            }

            u32 sender_id;
            f32 damage;
            f32 explosion_radius;
            f32 explosion_duration;
            f32 time_to_live;
    };

    class ParticleSystem : public Component
    {
        public:
            ParticleSystem(Emitter *emitter, f32 time_to_emit = 0.1f)
                : particles_to_be_emitted(0), time_to_emit(time_to_emit)
            {
                this->emitter = std::unique_ptr<Emitter>(emitter);
            }

            std::unique_ptr<Emitter> emitter;
            u32 particles_to_be_emitted;
            f32 time_to_emit;
    };
};  // namespace bls
