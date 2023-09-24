#include "stages/test_stage.hpp"
#include "core/game.hpp"
#include "core/input.hpp"
#include "ecs/systems.hpp"
#include "ecs/entities.hpp"
#include "ecs/scene_parser.hpp"
#include "ecs/state_machine.hpp"

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
        ecs->add_system(player_controller_system);
        ecs->add_system(state_machine_system);
        ecs->add_system(camera_system);
        ecs->add_system(animation_system);
        ecs->add_system(render_system);
        ecs->add_system(sound_system);

        // Load entities from file
        // SceneParser::parse_scene(*ecs, "bloss1/assets/scenes/test_stage.bloss");
        SceneParser::parse_scene(*ecs, "bloss1/assets/scenes/mecha_movement.bloss");

        // Create state machine for the player
        std::map<str, State*> states;
        auto idle_state = new IdleState();
        auto walking_state = new WalkingState();

        states[PLAYER_STATE_IDLE] = idle_state;
        states[PLAYER_STATE_WALKING] = walking_state;
        ecs->state_machines[0] = std::make_unique<StateMachine>(states, idle_state);
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
