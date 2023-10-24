#pragma once

/**
 * @brief The entity component system (ECS). Entities are containers of components,
 * components store the raw data and systems implement the necessary logic.
 */

#include "core/core.hpp"
#include "ecs/components.hpp"

#define MAX_ENTITY_ID 100005U

namespace bls
{
    // Forward declaration
    class Component;
    class ECS;

    // System: the logic bits
    typedef void (*System) (ECS& ecs, f32 dt);

    // ECS: container of the systems and entities
    class ECS
    {
        public:
            ECS(u32 max_entity_id = MAX_ENTITY_ID) : max_entity_id(max_entity_id)
            {
                for (u32 id = 0; id <= max_entity_id; id++)
                    available_ids.insert(id);
            }

            ~ECS()
            {

            }

            // Return a new id (create a new entity)
            u32 get_id()
            {
                if (available_ids.empty())
                    throw std::runtime_error("no available ids left");

                u32 id = *available_ids.begin();
                available_ids.erase(id);

                return id;
            }

            // Register a system
            void add_system(System system)
            {
                systems.push_back(system);
            }

            // Erase all the components of an entity (@TODO: this is not very efficient)
            void erase_entity(u32 id)
            {
                if (id > max_entity_id)
                    throw std::runtime_error("tried to delete invalid id: " + to_str(id));

                names.erase(id);
                transforms.erase(id);
                models.erase(id);
                dir_lights.erase(id);
                point_lights.erase(id);
                physics_objects.erase(id);
                colliders.erase(id);
                transform_animations.erase(id);
                timers.erase(id);
                cameras.erase(id);
                camera_controllers.erase(id);
                texts.erase(id);
                sounds.erase(id);
                state_machines.erase(id);
                projectiles.erase(id);

                available_ids.insert(id);
            }

            // Registered systems
            std::vector<System> systems;

            // Table of components
            std::map<u32, str> names;
            std::map<u32, std::unique_ptr<Transform>> transforms;
            std::map<u32, std::unique_ptr<ModelComponent>> models;
            std::map<u32, std::unique_ptr<DirectionalLight>> dir_lights;
            std::map<u32, std::unique_ptr<PointLight>> point_lights;
            std::map<u32, std::unique_ptr<PhysicsObject>> physics_objects;
            std::map<u32, std::unique_ptr<Collider>> colliders;
            std::map<u32, std::unique_ptr<TransformAnimation>> transform_animations;
            std::map<u32, std::unique_ptr<Timer>> timers;
            std::map<u32, std::unique_ptr<Camera>> cameras;
            std::map<u32, std::unique_ptr<CameraController>> camera_controllers;
            std::map<u32, std::unique_ptr<Text>> texts;
            std::map<u32, std::map<str, std::unique_ptr<Sound>>> sounds;
            std::map<u32, std::unique_ptr<StateMachine>> state_machines;
            std::map<u32, std::unique_ptr<Projectile>> projectiles;
            std::map<u32, f32> hitpoints;

        private:
            // Entities IDs
            u32 max_entity_id;
            std::set<u32> available_ids;
    };
};
