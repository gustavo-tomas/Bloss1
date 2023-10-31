#include "stages/test_stage.hpp"

#include "core/game.hpp"
#include "core/input.hpp"
#include "ecs/entities.hpp"
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
        ecs->add_system(player_controller_system);
        ecs->add_system(ophanim_controller_system);
        ecs->add_system(physics_system);
        ecs->add_system(projectile_system);
        ecs->add_system(state_machine_system);
        ecs->add_system(camera_system);
        ecs->add_system(animation_system);
        ecs->add_system(render_system);
        ecs->add_system(sound_system);

        // Load entities from file
        // SceneParser::parse_scene(*ecs, "bloss1/assets/scenes/test_stage.bloss");
        SceneParser::parse_scene(*ecs, "bloss1/assets/scenes/mecha_movement.bloss");

        auto &renderer = Game::get().get_renderer();
        renderer.create_shadow_map(*ecs);
        renderer.create_post_processing_passes();

        // Load configurations from file
        SceneParser::parse_scene(*ecs, "bloss1/assets/scenes/bloss_config.bcfg");

        const u32 player_id = 0;
        const u32 ophanim_id = 1;

        ecs->state_machines[player_id] = std::make_unique<StateMachine>(PLAYER_STATE_IDLE);
        ecs->state_machines[player_id]->state->enter(*ecs, player_id, ecs->state_machines[player_id]->current_state);

        ecs->state_machines[ophanim_id] = std::make_unique<StateMachine>(OPHANIM_STATE_IDLE);
        ecs->state_machines[ophanim_id]->state->enter(*ecs, ophanim_id, ecs->state_machines[ophanim_id]->current_state);

        auto *box_emitter = new BoxEmitter(ecs->transforms[player_id]->position, false, vec3(10.0f));
        ecs->particle_systems[player_id] = std::make_unique<ParticleSystem>(box_emitter);
    }

    void TestStage::update(f32 dt)
    {
        BLS_PROFILE_SCOPE("update");

        // Update all systems in registration order
        auto &systems = ecs->systems;
        for (const auto &system : systems) system(*ecs, dt);

        // @TODO: Player won
        if (ecs->hitpoints[1] <= 0.0f)
        {
            Game::get().change_stage(nullptr);
            return;
        }

        // @TODO: Player lost
        if (ecs->hitpoints[0] <= 0.0f)
        {
            Game::get().change_stage(nullptr);
            return;
        }

        // Exit the stage
        if (Input::is_key_pressed(KEY_ESCAPE))
        {
            Game::get().change_stage(nullptr);
            return;
        }
    }
};  // namespace bls
