#include "stages/test_stage.hpp"
#include "core/game.hpp"
#include "core/input.hpp"
#include "ecs/systems.hpp"
#include "ecs/entities.hpp"
#include "ecs/scene_parser.hpp"

namespace bls
{
    TestStage::TestStage()
    {

    }

    TestStage::~TestStage()
    {

    }

    void TestStage::start()
    {
        // Create the ECS
        ecs = std::unique_ptr<ECS>(new ECS());

        // Add systems in order of execution
        ecs->add_system(physics_system);
        ecs->add_system(camera_controller_system);
        ecs->add_system(camera_system);
        ecs->add_system(animation_system);
        ecs->add_system(render_system);
        ecs->add_system(sound_system);

        // Add some entities to the world
        player(*ecs, Transform(vec3(0.0f, 10.0f, 0.0f), vec3(0.0f, 90.0f, 0.0f), vec3(5.0f)));

        // Load entities from file
        SceneParser::parse_scene(*ecs, "bloss1/assets/scenes/test_stage.bloss");

        abomination(*ecs, Transform(vec3(-30.0f, 40.0f, 0.0f), vec3(-90.0f, 0.0f, 180.0f), vec3(1.0f, 1.0f, 1.0f))); // @TODO: fix rotation

        // Add some text
        text(*ecs,
             Transform(vec3(20.0f), vec3(0.0f), vec3(0.5f)),
             "Very high impact text",
             vec3(0.4f, 0.6f, 0.8f));

        // Add background music
        background_music(*ecs, Transform(), Sound("test", 0.5f, true), "bloss1/assets/sounds/test.wav");
    }

    void TestStage::update(f32 dt)
    {
        // Update all systems in registration order
        auto& systems = ecs->systems;
        for (auto& system : systems)
            system(*ecs, dt);

        // Exit the stage
        if (Input::is_key_pressed(KEY_ESCAPE))
            Game::get().change_stage(nullptr);
    }
};
