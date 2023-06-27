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
            ECS()
                : id_counter(0) { }

            ~ECS()
            {
                systems.clear();
                transforms.clear();
                models.clear();

                std::cout << "world destroyed successfully\n";
            }

            // Return a new id (create a new entity)
            u32 get_id()
            {
                if (id_counter >= MAX_ENTITY_ID)
                {
                    std::cerr << "id_counter reached maximum id\n";
                    exit(1);
                }
                return id_counter++;
            }

            // Register a system
            void add_system(System system)
            {
                systems.push_back(system);
            }

            // Registered systems
            std::vector<System> systems;

            // Table of components
            // @TODO: use templates or smth
            std::map<u32, std::unique_ptr<Transform>> transforms;
            std::map<u32, std::unique_ptr<Mesh>> models;

        private:
            // Entities IDs
            u32 id_counter;
    };
};
