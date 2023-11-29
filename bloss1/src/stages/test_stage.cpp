#include "stages/test_stage.hpp"

#include "core/game.hpp"
#include "core/input.hpp"
#include "ecs/entities.hpp"
#include "ecs/scene_parser.hpp"
#include "ecs/state_machine.hpp"
#include "ecs/systems.hpp"
#include "stages/menu_stage.hpp"
#include "tools/profiler.hpp"

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
        ecs->add_system(player_controller_system);
        ecs->add_system(ophanim_controller_system);
        ecs->add_system(physics_system);
        ecs->add_system(bullet_indicator_system);
        ecs->add_system(projectile_system);
        ecs->add_system(state_machine_system);
        ecs->add_system(camera_system);
        ecs->add_system(animation_system);
        ecs->add_system(render_system_forward);  // 8
        ecs->add_system(sound_system);
        ecs->add_system(cleanup_system);

        // Load entities from file
        SceneParser::parse_scene(*ecs, "bloss1/assets/scenes/main_stage.bloss");

        auto &renderer = Game::get().get_renderer();
        if (renderer.get_shadow_map() == nullptr)
        {
            renderer.create_shadow_map(*ecs);
            renderer.create_height_map(2048, 2048, 4, 64, 20.0f, 1000.0f);
            renderer.create_post_processing_passes(*ecs);
        }

        // Load configurations from file
        SceneParser::parse_scene(*ecs, "bloss1/assets/scenes/bloss_config.bcfg");
    }

    void TestStage::update(f32 dt)
    {
        BLS_PROFILE_SCOPE("update");

        // Exit the stage
        if (Input::is_key_pressed(KEY_ESCAPE))
        {
            Game::get().change_stage(new MenuStage());
            return;
        }

        // Update all systems in registration order
        auto &systems = ecs->systems;
        for (const auto &system : systems) system(*ecs, dt);

        if (ecs->systems.size() == 0) return;

        // @TODO: Player won
        if (ecs->hitpoints[1] <= 0.0f)
        {
            auto &audio_engine = Game::get().get_audio_engine();

            audio_engine.load("ophanim_death_sfx", "bloss1/assets/sounds/124601__nominal__nog-paal.wav");
            audio_engine.play("ophanim_death_sfx");

            Game::get().change_stage(new MenuStage());
            return;
        }

        // @TODO: Player lost
        else if (ecs->hitpoints[0] <= 0.0f)
        {
            auto &audio_engine = Game::get().get_audio_engine();

            audio_engine.load("player_death_sfx",
                              "bloss1/assets/sounds/505751__thehorriblejoke__computer-breaking-sound.wav");
            audio_engine.play("player_death_sfx");

            Game::get().change_stage(new MenuStage());
            return;
        }
    }
};  // namespace bls
