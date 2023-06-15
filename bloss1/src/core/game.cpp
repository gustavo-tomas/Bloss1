#include "core/game.hpp"
#include "ecs/systems.hpp"
#include "ecs/entities.hpp"

namespace bls
{
    Game* Game::instance = nullptr;

    Game::Game(const str& title, const u32& width, const u32& height)
    {
        // Create game instance
        if (instance != nullptr)
        {
            std::cerr << "there can be only one instance of game\n";
            exit(1);
        }

        instance = this;

        // Create a window
        window = Window::create(title, width, height);

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

        running = true;
    }

    Game::~Game()
    {
        delete ecs;
        delete window;

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
        // Time variation
        f64 last_time = window->get_time(), current_time = 0, dt = 0;

        // The game loop
        while (running)
        {
            // Calculate dt
            current_time = window->get_time();
            dt = current_time - last_time;
            last_time = current_time;

            std::cout << "DT: " << dt << "\n";

            // Update all systems in registration order
            auto& systems = ecs->systems;
            for (auto& system : systems)
                system(*ecs, dt);

            // Update window
            window->update();
        }
    }
};
