#include "game.hpp"
#include "ecs/systems.hpp"

namespace bls
{
    game* create_game(const str& title, const u32& width, const u32& height)
    {
        game* gm = new game(title, width, height);
        gm->wrld = create_world();

        world* my_world = gm->wrld;

        add_system(*my_world, transform_system);
        add_system(*my_world, render_system);

        entity* my_entity = create_entity(*my_world);

        transform* my_transform = create_component<transform>(10, 20);
        model* my_model = create_component<model>(200);

        add_component_to_entity(*my_entity, "transform", *my_transform);
        add_component_to_entity(*my_entity, "model", *my_model);

        return gm;
    }

    void destroy_game(const game& gm)
    {
        delete &gm;
    }

    void update_game(const game& gm)
    {
        update_world(*gm.wrld, 0.1f);
    }
};
