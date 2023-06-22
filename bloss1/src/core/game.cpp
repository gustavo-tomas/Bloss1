#include "core/game.hpp"
#include "renderer/factory.hpp"
#include "stages/test_stage.hpp"

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
        window->set_event_callback(BIND_EVENT_FN(Game::on_event));

        // Create a renderer
        renderer = std::unique_ptr<Renderer>(RendererFactory::create_renderer(BackendType::OpenGL));
        renderer->initialize();

        // Register callbacks
        EventSystem::register_callback<WindowCloseEvent>(BIND_EVENT_FN(Game::on_window_close));
        EventSystem::register_callback<WindowResizeEvent>(BIND_EVENT_FN(Game::on_window_resize));
        EventSystem::register_callback<KeyPressEvent>(BIND_EVENT_FN(Game::on_key_press));
        EventSystem::register_callback<MouseScrollEvent>(BIND_EVENT_FN(Game::on_mouse_scroll));

        // Register initial stage
        stages = std::unique_ptr<Stage>(new TestStage(*renderer.get())); // oof
        stages->start();

        running = true;
        minimized = false;
    }

    Game::~Game()
    {
        std::cout << "game destroyed successfully\n";
    }

    void Game::run()
    {
        // Time variation
        f64 last_time = window->get_time(), current_time = 0, dt = 0;

        // The game loop
        while (running)
        {
            // Update ----------------------------------------------------------
            // Don't render if the application is minimized
            if (minimized)
            {
                window->update();
                continue;
            }

            if (!stages->is_running())
                running = false;

            // Calculate dt
            current_time = window->get_time();
            dt = current_time - last_time;
            last_time = current_time;

            stages->update(dt);

            // Render ----------------------------------------------------------
            // @TODO: batching ?
            stages->render();

            // Update window
            window->update();
        }
    }

    void Game::push_stage(Stage* stage)
    {
        stages = std::unique_ptr<Stage>(stage);
    }

    void Game::on_event(Event& event)
    {
        if (typeid(event) == typeid(WindowCloseEvent))
            EventSystem::fire_event(static_cast<const WindowCloseEvent&>(event));

        else if (typeid(event) == typeid(WindowResizeEvent))
            EventSystem::fire_event(static_cast<const WindowResizeEvent&>(event));

        else if (typeid(event) == typeid(KeyPressEvent))
            EventSystem::fire_event(static_cast<const KeyPressEvent&>(event));

        else if (typeid(event) == typeid(MouseScrollEvent))
            EventSystem::fire_event(static_cast<const MouseScrollEvent&>(event));
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

    Renderer& Game::get_renderer()
    {
        return *renderer;
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
        if (event.width <= 100 || event.height <= 100)
            minimized = true;

        else
            minimized = false;
    }
};
