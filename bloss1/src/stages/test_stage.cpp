#include "stages/test_stage.hpp"
#include "core/game.hpp"
#include "core/input.hpp"
#include "ecs/systems.hpp"
#include "ecs/entities.hpp"

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

        for (u32 i = 0; i < 5; i++)
            ball(*ecs, Transform(vec3((i + 1) * 10.0f, 10.0f, 0.0f), vec3(0.0f, 90.0f, 0.0f), vec3(5.0f)));

        vampire(*ecs, Transform(vec3(-20.0f, 10.0f, -20.0f), vec3(0.0f, 180.0f, 0.0f), vec3(0.001f, 0.001f, 0.001f)));
        abomination(*ecs, Transform(vec3(-30.0f, 40.0f, 0.0f), vec3(-90.0f, 0.0f, 180.0f), vec3(1.0f, 1.0f, 1.0f))); // @TODO: fix rotation

        // Floor is created last
        floor(*ecs, Transform(vec3(0.0f), vec3(0.0f), vec3(10.0f, 1.0f, 10.0f)));

        // Add directional lights
        directional_light(*ecs,
                          Transform(vec3(0.0f), vec3(0.3f, -1.0f, 0.15f)),
                          DirectionalLight(vec3(0.0f), vec3(0.005f, 0.005f, 0.005f)));

        // Add point lights
        point_light(*ecs, Transform(vec3( 100.0f, 100.0f,  100.0f)), PointLight(vec3(40000.0f)));
        point_light(*ecs, Transform(vec3( 100.0f, 100.0f, -100.0f)), PointLight(vec3(40000.0f)));
        point_light(*ecs, Transform(vec3(-100.0f, 100.0f,  100.0f)), PointLight(vec3(40000.0f)));
        point_light(*ecs, Transform(vec3(-100.0f, 100.0f, -100.0f)), PointLight(vec3(40000.0f)));

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
