#include "stages/test_stage.hpp"
#include "core/game.hpp"
#include "core/input.hpp"
#include "ecs/systems.hpp"
#include "ecs/entities.hpp"
#include "ecs/scene_parser.hpp"
#include "ecs/state_machine.hpp"
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

        // @TODO: Create state machine for the player
        std::map<str, State*> states;
        auto idle_state = new PlayerIdleState();
        auto walking_state = new PlayerWalkingState();
        // auto jumping_state = new PlayerJumpingState();
        auto shooting_state = new PlayerShootingState();

        states[PLAYER_STATE_IDLE]     = idle_state;
        states[PLAYER_STATE_WALKING]  = walking_state;
        // states[PLAYER_STATE_JUMPING]  = jumping_state;
        states[PLAYER_STATE_SHOOTING] = shooting_state;

        idle_state->enter(*ecs, 0);
        ecs->state_machines[0] = std::make_unique<StateMachine>(states, idle_state);

        // @TODO: Create state machine for ophanim
        std::map<str, State*> ophanim_states;
        auto ophanim_idle_state = new OphanimIdleState();

        ophanim_states[OPHANIM_STATE_IDLE] = ophanim_idle_state;

        ophanim_idle_state->enter(*ecs, 1);
        ecs->state_machines[1] = std::make_unique<StateMachine>(ophanim_states, ophanim_idle_state);
    }

    void TestStage::update(f32 dt)
    {
        BLS_PROFILE_SCOPE("update");

        // Update all systems in registration order
        auto& systems = ecs->systems;
        for (auto& system : systems)
            system(*ecs, dt);

        // Exit the stage
        if (Input::is_key_pressed(KEY_ESCAPE))
            Game::get().change_stage(nullptr);
    }
};
