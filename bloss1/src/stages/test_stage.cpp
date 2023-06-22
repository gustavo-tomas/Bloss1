#include "stages/test_stage.hpp"
#include "core/input.hpp"
#include "ecs/systems.hpp"
#include "ecs/entities.hpp"

// Testing
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

namespace bls
{
    TestStage::TestStage(Renderer& renderer, Window& window) : renderer(renderer), window(window)
    {

    }

    TestStage::~TestStage()
    {
        delete controller;

        delete vao;
        delete vbo;
        delete ebo;
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

        vao = VertexArray::create();
        vao->bind();

        vbo = VertexBuffer::create(quadVertices, sizeof(quadVertices));
        vbo->bind();
        vao->add_vertex_buffer(0, 2, ShaderDataType::Float, false, 2 * sizeof(f32), (void*) 0);

        ebo = IndexBuffer::create(indices, 6);
        ebo->bind();

        shader = Shader::create("test", "bloss1/assets/shaders/test.vs", "bloss1/assets/shaders/test.fs");

        running = true;
    }

    void TestStage::update(f32 dt)
    {
        if (!running)
            return;

        // Update camera controller
        controller->update(dt);

        // Update all systems in registration order
        auto& systems = ecs->systems;
        for (auto& system : systems)
            system(*ecs, dt);

        if (Input::is_key_pressed(KEY_ESCAPE))
            running = false;
    }

    void TestStage::render()
    {
        // Window properties
        auto width = window.get_width();
        auto height = window.get_height();

        // Camera properties
        auto projection = controller->get_camera().get_projection_matrix(width, height);
        auto view = controller->get_camera().get_view_matrix();

        // Clear the screen
        renderer.clear();
        renderer.clear_color({ 0.4f, 0.6f, 0.8f, 1.0f });

        // Bind and update data to shader
        shader->bind();
        shader->set_uniform3("color", { 0.8f, 0.6f, 0.4f });
        shader->set_uniform4("model", mat4(1.0f));
        shader->set_uniform4("projection", projection);
        shader->set_uniform4("view", view);
        vao->bind();

        renderer.draw_indexed(sizeof(indices));
    }

    bool TestStage::is_running()
    {
        return running;
    }
};
