#pragma once

/**
 * @brief The entity component system (ECS). Entities are containers of components,
 * components store the raw data and systems implement the necessary logic.
 */

#include "core/core.hpp"

namespace bls
{
    // Forward declaration
    struct component;
    struct entity;
    struct world;

    // Component: contain the data
    struct component
    {
        virtual ~component() { };
    };

    // Entity: container of components
    struct entity
    {
        ~entity()
        {
            for (auto& [name, cmp] : components)
                delete cmp;

            components.clear();
        }

        std::map<str, component*> components = { };
    };

    // System: the logic bits
    typedef void (*system) (const world& wrld, const f32 dt);

    // World: container of the systems and entities
    struct world
    {
        ~world()
        {
            systems.clear();
            entities.clear();

            std::cout << "world destroyed successfully\n";
        }

        std::vector<system> systems = { };
        std::vector<std::unique_ptr<entity>> entities = { };
    };

    world* create_world();
    entity* create_entity(world& wrld);

    template<typename T, typename... Args>
    T* create_component(Args... args) { return new T(args...); }

    void add_system(world& wrld, system sys);
    void add_component_to_entity(entity& ent, const str& name, component& cmp);

    // Query entities with the required components
    std::vector<entity*> query_entities(const world& wrld, const std::vector<str>& query);

    // Update all the registered systems
    void update_world(const world& wrld, const f32 dt);
};
