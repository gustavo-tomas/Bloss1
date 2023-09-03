#include "stages/test_stage.hpp"
#include "core/input.hpp"
#include "ecs/systems.hpp"
#include "ecs/entities.hpp"
#include "core/game.hpp"

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
        ecs->add_system(camera_controller_system);
        ecs->add_system(camera_system);
        ecs->add_system(animation_system);
        ecs->add_system(physics_system);
        ecs->add_system(render_system);

        // Add some entities to the world
        player(*ecs, Transform(vec3(0.0f, 50.0f, 0.0f), vec3(0.0f, 90.0f, 0.0f), vec3(5.0f)));

        for (u32 i = 0; i < 5; i++)
            ball(*ecs, Transform(vec3((i + 1) * 10.0f, 50.0f, 0.0f), vec3(0.0f, 90.0f, 0.0f), vec3(5.0f)));

        vampire(*ecs, Transform(vec3(-20.0f, 50.0f, -20.0f), vec3(0.0f), vec3(0.001f, 0.001f, 0.001f)));
        abomination(*ecs, Transform(vec3(-30.0f, 40.0f, 0.0f), vec3(-90.0f, 0.0f, 180.0f), vec3(1.0f, 1.0f, 1.0f))); // @TODO: fix rotation

        // Floor is created last
        floor(*ecs, Transform(vec3(0.0f), vec3(0.0f), vec3(10.0f, 1.0f, 10.0f)));

        // Add directional lights
        directional_light(*ecs,
                          Transform(vec3(0.0f), vec3(0.3f, -1.0f, 0.15f)),
                          DirectionalLight(vec3(0.2f), vec3(1.0f), vec3(1.0f)));

        // Add point lights
        point_light(*ecs, Transform(vec3( 100.0f, 100.0f,  100.0f)), PointLight(vec3(40000.0f)));
        point_light(*ecs, Transform(vec3( 100.0f, 100.0f, -100.0f)), PointLight(vec3(40000.0f)));
        point_light(*ecs, Transform(vec3(-100.0f, 100.0f,  100.0f)), PointLight(vec3(40000.0f)));
        point_light(*ecs, Transform(vec3(-100.0f, 100.0f, -100.0f)), PointLight(vec3(40000.0f)));

        // Load audios
        // auto& audio_engine = Game::get().get_audio_engine();
        // audio_engine.load("test", "bloss1/assets/sounds/toc.wav", false);
        // audio_engine.set_echo_filter("test", 0.2f, 0.15f);
        // audio_engine.play("test");

        // Create a video player
        // video_player = std::make_unique<VideoPlayer>("bloss1/assets/videos/mh_pro_skate.mp4");
        // video_player->play_video();

        running = true;
    }

    void TestStage::update(f32 dt)
    {
        // Update all systems in registration order
        auto& systems = ecs->systems;
        for (auto& system : systems)
            system(*ecs, dt);

        // Exit the stage
        if (Input::is_key_pressed(KEY_ESCAPE))
            running = false;
    }

    bool TestStage::is_running()
    {
        return running;
    }
};
