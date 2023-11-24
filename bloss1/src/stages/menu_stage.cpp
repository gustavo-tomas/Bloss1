#include "stages/menu_stage.hpp"

#include "core/game.hpp"
#include "core/input.hpp"
#include "ecs/scene_parser.hpp"
#include "ecs/systems.hpp"
#include "stages/test_stage.hpp"

namespace bls
{
    MenuStage::MenuStage()
    {
    }

    MenuStage::~MenuStage()
    {
    }

    void MenuStage::start()
    {
        // Create the ECS
        ecs = std::unique_ptr<ECS>(new ECS());

        // Add systems in order of execution
        ecs->add_system(ophanim_controller_system);
        ecs->add_system(state_machine_system);
        ecs->add_system(camera_system);
        ecs->add_system(render_system_forward);

        // Load entities from file
        SceneParser::parse_scene(*ecs, "bloss1/assets/scenes/menu.bloss");

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
            renderer.create_height_map(2048, 2048, 4, 64, 20.0f, 1000.0f);
            renderer.create_post_processing_passes(*ecs);
        }

        // Load configurations from file
        SceneParser::parse_scene(*ecs, "bloss1/assets/scenes/menu_config.bcfg");

        const u32 ophanim_id = 1;

        ecs->state_machines[ophanim_id] = std::make_unique<StateMachine>(OPHANIM_STATE_IDLE);
        ecs->state_machines[ophanim_id]->state->enter(*ecs, ophanim_id, ecs->state_machines[ophanim_id]->current_state);
    }

    void MenuStage::update(f32 dt)
    {
        // Update all systems in registration order
        auto &systems = ecs->systems;
        for (const auto &system : systems) system(*ecs, dt);

        if (Input::is_key_pressed(KEY_ESCAPE) || Input::is_joystick_button_pressed(JOYSTICK_2, GAMEPAD_BUTTON_CIRCLE))
            Game::get().change_stage(nullptr);

        if (Input::is_key_pressed(KEY_SPACE) || Input::is_joystick_button_pressed(JOYSTICK_2, GAMEPAD_BUTTON_CROSS))
            Game::get().change_stage(new TestStage());
    }
};  // namespace bls
