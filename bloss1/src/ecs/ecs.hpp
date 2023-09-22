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
            ECS(unsigned int max_entity_id = MAX_ENTITY_ID) : max_entity_id(max_entity_id)
            {
                for (auto id = 0; id <= max_entity_id; id++)
                    available_ids.insert(id);
            }

            ~ECS()
            {
                systems.clear();
                transforms.clear();
                models.clear();
                dir_lights.clear();

                std::cout << "world destroyed successfully\n";
            }

            // Return a new id (create a new entity)
            u32 get_id()
            {
                if (available_ids.empty())
                {
                    std::cerr << "no available ids left\n";
                    exit(1);
                }

                int id = *available_ids.begin();
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
                {
                    std::cerr << "tried to delete invalid id: " << id << "\n";
                    exit(1);
                }

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

                available_ids.insert(id);
            }

            // Registered systems
            std::vector<System> systems;

            // Table of components
            // @TODO: use templates or smth
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

        private:
            // Entities IDs
            unsigned int max_entity_id;
            std::set<unsigned int> available_ids;
    };
};
