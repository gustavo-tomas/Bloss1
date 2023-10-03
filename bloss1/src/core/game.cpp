#include "core/game.hpp"
#include "core/logger.hpp"
// #include "stages/menu_stage.hpp"
#include "stages/test_stage.hpp"
#include "tools/profiler.hpp"

namespace bls
{
    Game* Game::instance = nullptr;

    Game::Game(const str& title, const u32& width, const u32& height)
    {
        // Create game instance
        if (instance != nullptr)
            throw std::runtime_error("there can be only one instance of game");

        instance = this;

        // Create the window
        window = std::unique_ptr<Window>(Window::create(title, width, height));
        window->set_event_callback(BIND_EVENT_FN(Game::on_event));

        // Create the renderer
        renderer = std::unique_ptr<Renderer>(Renderer::create());

        // Create the editor
        #if !defined(_RELEASE)
        editor = std::make_unique<Editor>(*window.get());
        #endif

        // Create the audio engine
        audio_engine = std::unique_ptr<AudioEngine>(AudioEngine::create());

        // Create RNG engine
        random_engine = std::make_unique<Random>();

        // Register callbacks
        EventSystem::register_callback<WindowCloseEvent>(BIND_EVENT_FN(Game::on_window_close));
        EventSystem::register_callback<WindowResizeEvent>(BIND_EVENT_FN(Game::on_window_resize));
        EventSystem::register_callback<KeyPressEvent>(BIND_EVENT_FN(Game::on_key_press));
        EventSystem::register_callback<MouseScrollEvent>(BIND_EVENT_FN(Game::on_mouse_scroll));
    }

    Game::~Game()
    {

    }

    void Game::run()
    {
        BLS_PROFILE_BEGIN_SESSION("MainLoop", "profile/runtime.json");

        // Register initial stage
        change_stage(new TestStage());

        // Initialize after the ecs
        renderer->initialize();

        // Time variation
        f64 last_time = window->get_time(), current_time = 0, dt = 0;

        set_target_fps(0);

        window_open = true;
        minimized = false;

        // The game loop
        while (stage && window_open)
        {
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

            // Update running stage
            stage->update(dt);

            if (!stage)
                break;

            // Update editor
            #if !defined(_RELEASE)
            editor->update(*stage->ecs, dt);
            #endif

            // Update window
            window->update();

            // Sleep to match target spf
            f64 elapsed = window->get_time() - last_time;
            if (target_spf - elapsed > 0.0)
                window->sleep(target_spf - elapsed);
        }

        BLS_PROFILE_END_SESSION();
    }

    void Game::change_stage(Stage* new_stage)
    {
        stage.reset();
        stage = std::unique_ptr<Stage>(new_stage);
        if (stage)
            stage->start();
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

        else if (typeid(event) == typeid(MouseMoveEvent))
            EventSystem::fire_event(static_cast<const MouseMoveEvent&>(event));

        else
            LOG_ERROR("invalid event type");
    }

    Game& Game::get()
    {
        if (instance == nullptr)
            throw std::runtime_error("game instance is nullptr");

        return *instance;
    }

    Renderer& Game::get_renderer()
    {
        return *renderer;
    }

    AudioEngine& Game::get_audio_engine()
    {
        return *audio_engine;
    }

    Window& Game::get_window()
    {
        return *window;
    }

    Random& Game::get_random_engine()
    {
        return *random_engine;
    }

    Stage& Game::get_curr_stage()
    {
        return *stage;
    }

    void Game::set_target_fps(u32 fps)
    {
        fps = (fps == 0) ? 100'000 : fps;
        target_spf = 1.0 / static_cast<f64>(fps);
    }

    void Game::on_window_close(const WindowCloseEvent&)
    {
        window_open = false;
    }

    void Game::on_key_press(const KeyPressEvent&)
    {

    }

    void Game::on_mouse_scroll(const MouseScrollEvent&)
    {

    }

    void Game::on_window_resize(const WindowResizeEvent& event)
    {
        if (event.width <= 100 || event.height <= 100)
            minimized = true;

        else
            minimized = false;
    }
};
