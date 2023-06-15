#include "core/game.hpp"
#include "core/input.hpp"
#include "core/key_codes.hpp"
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
        window = std::unique_ptr<Window>(Window::create(title, width, height));

        // Create ECS
        ecs = new ECS();

        // Add systems in order of execution
        ecs->add_system(transform_system);
        ecs->add_system(render_system);

        // Add some entities
        u32 e1 = player(*ecs, Transform(10, 20, 30));
        u32 e2 = player(*ecs, Transform(100, 200, 300));

        std::cout << "e1: " << e1 << " e2: " << e2 << "\n";

        // Register callbacks
        EventSystem::register_callback<WindowCloseEvent>(BIND_EVENT_FN(Game::on_window_close));
        EventSystem::register_callback<WindowResizeEvent>(BIND_EVENT_FN(Game::on_window_resize));
        EventSystem::register_callback<KeyPressEvent>(BIND_EVENT_FN(Game::on_key_press));
        EventSystem::register_callback<MouseScrollEvent>(BIND_EVENT_FN(Game::on_mouse_scroll));

        running = true;
    }

    Game::~Game()
    {
        delete ecs;

        std::cout << "game destroyed successfully\n";
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

            // Update all systems in registration order
            auto& systems = ecs->systems;
            for (auto& system : systems)
                system(*ecs, dt);

            if (Input::is_key_pressed(KEY_ESCAPE))
                running = false;

            // Update window
            window->update();
        }
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

    Window& Game::get_window()
    {
        return *window;
    }

    void Game::on_window_close(const WindowCloseEvent&)
    {
        running = false;
    }

    void Game::on_key_press(const KeyPressEvent& event)
    {
        std::cout << "Key pressed: " << event.key << "\n";
    }

    void Game::on_mouse_scroll(const MouseScrollEvent& event)
    {
        std::cout << "Scroll X: " << event.xoffset << " Scroll Y: " << event.yoffset << "\n";
    }

    void Game::on_window_resize(const WindowResizeEvent& event)
    {
        std::cout << "Width: " << event.width << " Height: " << event.height << "\n";
    }
};
