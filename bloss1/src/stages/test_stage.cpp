#include "stages/test_stage.hpp"

#include "core/game.hpp"
#include "core/input.hpp"
#include "ecs/scene_parser.hpp"
#include "ecs/state_machine.hpp"
#include "ecs/systems.hpp"
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
        ecs->add_system(physics_system);
        ecs->add_system(state_machine_system);
        ecs->add_system(camera_system);
        ecs->add_system(animation_system);
        ecs->add_system(render_system_deferred);
        ecs->add_system(cleanup_system);

        // Load entities from file
        SceneParser::parse_scene(*ecs, "bloss1/assets/scenes/test_stage.bloss");

        auto &renderer = Game::get().get_renderer();
        if (renderer.get_shadow_map() == nullptr)
        {
            renderer.create_skybox("bloss1/assets/textures/satara_night_no_lamps_4k.hdr",
                                   AppConfig::skybox_config.skybox_resolution,
                                   AppConfig::skybox_config.irradiance_resolution,
                                   AppConfig::skybox_config.brdf_resolution,
                                   AppConfig::skybox_config.prefilter_resolution,
                                   AppConfig::skybox_config.max_mip_levels);

            renderer.create_shadow_map(*ecs);
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
            Game::get().change_stage(nullptr);
            return;
        }

        // Update all systems in registration order
        auto &systems = ecs->systems;
        for (const auto &system : systems) system(*ecs, dt);
    }
};  // namespace bls
