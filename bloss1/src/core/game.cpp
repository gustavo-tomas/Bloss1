#include "core/game.hpp"
#include "core/input.hpp"
#include "core/key_codes.hpp"
#include "math/math.hpp"
#include "renderer/factory.hpp"
#include "managers/shader_manager.hpp"
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
        window->set_event_callback(BIND_EVENT_FN(Game::on_event));

        // Create a renderer
        renderer = std::unique_ptr<Renderer>(RendererFactory::create_renderer(BackendType::OpenGL));
        renderer->initialize();

        auto arr = renderer->create_vertex_array();
        delete arr;

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
        minimized = false;
    }

    Game::~Game()
    {
        delete ecs;

        std::cout << "game destroyed successfully\n";
    }

    void Game::run()
    {
        f32 quadVertices[] =
        {
            -0.5f, -0.5f, // 0
            0.5f, -0.5f,  // 1
            0.5f, 0.5f,   // 2
            -0.5f, 0.5f,  // 3
        };

        u32 indices[] =
        {
            0, 1, 2,  // first Triangle
            2, 3, 0   // second Triangle
        };

        auto vao = renderer->create_vertex_array();
        vao->bind();

        auto vbo = renderer->create_vertex_buffer(quadVertices, sizeof(quadVertices));
        vbo->bind();
        vao->add_vertex_buffer(0, 2, ShaderDataType::Float, false, 2 * sizeof(f32), (void*) 0);

        auto ebo = renderer->create_index_buffer(indices, 6);
        ebo->bind();

        auto shader = ShaderManager::get().load("test", "bloss1/assets/shaders/test.vs", "bloss1/assets/shaders/test.fs");

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

            // Calculate dt
            current_time = window->get_time();
            dt = current_time - last_time;
            last_time = current_time;

            // Update all systems in registration order
            auto& systems = ecs->systems;
            for (auto& system : systems)
                system(*ecs, dt);

            // Input polling test
            if (Input::is_key_pressed(KEY_ESCAPE))
                running = false;

            // Render ----------------------------------------------------------
            renderer->clear();
            renderer->clear_color({ 0.4f, 0.6f, 0.8f, 1.0f });

            shader->bind();
            shader->set_uniform3("color", { 0.8f, 0.6f, 0.4f });
            vao->bind();

            renderer->draw_indexed(sizeof(indices));

            // Update window
            window->update();
        }

        delete vao;
        delete vbo;
        delete ebo;
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
