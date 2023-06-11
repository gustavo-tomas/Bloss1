#include "ecs.hpp"

namespace bls
{
    world* create_world()
    {
        return new world();
    }

    entity* create_entity(world& wrld)
    {
        auto& ent = wrld.entities.emplace_back(std::make_unique<entity>());
        return ent.get();
    }

    void add_system(world& wrld, system sys)
    {
        wrld.systems.push_back(sys);
    }

    void add_component_to_entity(entity& ent, const str& name, component& cmp)
    {
        auto& components = ent.components;

        // Add component to the entity if not already present
        if (!components.count(name))
            components[name] = &cmp;
    }

    std::vector<entity*> query_entities(const world& wrld, const std::vector<str>& query)
    {
        std::vector<entity*> filtered_entities;

        for (const auto& entity : wrld.entities)
        {
            bool has_all_components = true;

            // Check if the entity has all the required components
            for (const auto& name : query)
            {
                if (!entity->components.count(name))
                {
                    has_all_components = false;
                    break;
                }
            }

            if (has_all_components)
                filtered_entities.push_back(entity.get());
        }

        return filtered_entities;
    }

    void update_world(const world& wrld, const f32 dt)
    {
        for (const auto& sys : wrld.systems)
            sys(wrld, dt);
    }
};
