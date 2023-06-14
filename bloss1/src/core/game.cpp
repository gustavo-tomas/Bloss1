#include "core/game.hpp"
#include "ecs/systems.hpp"
#include "ecs/entities.hpp"

namespace bls
{
    Game* Game::instance = nullptr;

    Game::Game(const str& title, const u32& width, const u32& height)
    {
        if (instance != nullptr)
        {
            std::cerr << "there can be only one instance of game\n";
            exit(1);
        }

        instance = this;

        this->title = title;
        this->width = width;
        this->height = height;

        // Create ECS
        ecs = new ECS();

        // Add systems in order of execution
        ecs->add_system(transform_system);
        ecs->add_system(render_system);

        // Add some entities
        u32 e1 = player(*ecs, Transform(10, 20, 30));
        u32 e2 = player(*ecs, Transform(100, 200, 300));

        std::cout << "e1: " << e1 << " e2: " << e2 << "\n";
    }

    Game::~Game()
    {
        delete ecs;

        std::cout << "game destroyed successfully\n";
    }

    Game& Game::get()
    {
        if (instance == nullptr)
        {
            std::cerr << "game instance is nullptr\n";
            exit(1);
        }

        return *instance;
    }

    void Game::run()
    {
        // The game loop
        for (int i = 0; i < 5; i++)
        {
            // Update all registered systems in registration order
            auto& systems = ecs->systems;
            for (auto& system : systems)
                system(*ecs, dt);
        }
    }
};
