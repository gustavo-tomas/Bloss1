#include "stages/test_stage.hpp"
#include "core/input.hpp"
#include "ecs/systems.hpp"
#include "ecs/entities.hpp"

namespace bls
{
    TestStage::TestStage(Renderer& renderer, Window& window) : renderer(renderer), window(window)
    {

    }

    TestStage::~TestStage()
    {
        delete controller;
        delete cube;
    }

    void TestStage::start()
    {
        // Create a camera
        controller = new CameraController();

        // Create ECS
        ecs = std::unique_ptr<ECS>(new ECS());

        // Add systems in order of execution
        ecs->add_system(transform_system);
        ecs->add_system(render_system);

        // Add some entities
        u32 e1 = player(*ecs, Transform(10, 20, 30));
        u32 e2 = player(*ecs, Transform(100, 200, 300));

        std::cout << "e1: " << e1 << " e2: " << e2 << "\n";

        cube = new Cube(renderer);

        shader = Shader::create("test", "bloss1/assets/shaders/test.vs", "bloss1/assets/shaders/test.fs");

        running = true;
    }

    void TestStage::update(f32 dt)
    {
        // Update camera controller
        controller->update(dt);

        // Update all systems in registration order
        auto& systems = ecs->systems;
        for (auto& system : systems)
            system(*ecs, dt);

        // Window properties
        auto width = window.get_width();
        auto height = window.get_height();

        // Camera properties
        auto projection = controller->get_camera().get_projection_matrix(width, height);
        auto view = controller->get_camera().get_view_matrix();

        // Clear the screen
        renderer.clear();
        renderer.clear_color({ 0.4f, 0.6f, 0.8f, 1.0f });

        // Translation @TODO: next step: use the component system here
        vec3 position = { 1.0f, 5.0f, -3.0f };
        auto translation = translate(mat4(1.0f), position);

        // Bind and update data to shader
        shader->bind();
        shader->set_uniform3("color", { 0.8f, 0.6f, 0.4f });
        shader->set_uniform4("model", translation);
        shader->set_uniform4("projection", projection);
        shader->set_uniform4("view", view);

        cube->Render();

        // Exit the stage
        if (Input::is_key_pressed(KEY_ESCAPE))
            running = false;
    }

    bool TestStage::is_running()
    {
        return running;
    }
};
