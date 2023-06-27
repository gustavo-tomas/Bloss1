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

        // Add some entities to the world
        player(*ecs, Transform(vec3( 5.0f)));
        player(*ecs, Transform(vec3(-5.0f)));

        tex_shader = Shader::create("tex", "bloss1/assets/shaders/test/texture.vs", "bloss1/assets/shaders/test/texture.fs");
        texture = Texture::create("simple_texture", "bloss1/assets/textures/bark.png", TextureType::Diffuse);

        running = true;
    }

    void TestStage::update(f32 dt)
    {
        // Update camera controller
        controller->update(dt);

        // Update all systems in registration order
        // @TODO: for now do nothing
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

        // Render all entities
        for (const auto& [id, transform] : ecs->transforms)
        {
            auto scale_mat = scale(mat4(1.0f), transform->scale);
            auto translation_mat = translate(mat4(1.0f), transform->position);

            // Rotate
            auto pitch_quat = angle_axis(radians(transform->rotation.x), vec3(1.0f, 0.0f, 0.0f));
            auto yaw_quat = angle_axis(radians(transform->rotation.y), vec3(0.0f, 1.0f, 0.0f));
            auto roll_quat = angle_axis(radians(transform->rotation.z), vec3(0.0f, 0.0f, 1.0f));
            auto rotation_quat = normalize(yaw_quat * pitch_quat * roll_quat);
            auto rotation_mat = to_mat4(rotation_quat);

            // Remember: scale -> rotate -> translate
            auto model_matrix = translation_mat * rotation_mat * scale_mat;

            // Bind and update data to shader
            tex_shader->bind();
            tex_shader->set_uniform1("simple_texture", 0U); // Texture slot 0 (doesn't need to be called every frame)
            tex_shader->set_uniform3("color", { 0.8f, 0.2f, 0.1f });
            tex_shader->set_uniform4("model", model_matrix);
            tex_shader->set_uniform4("projection", projection);
            tex_shader->set_uniform4("view", view);

            // Set textures
            texture->bind(0);

            // Render the model
            auto model = ecs->models[id].get();
            model->vao->bind();
            renderer.draw_indexed(model->indices.size());
            model->vao->unbind();
        }

        // Exit the stage
        if (Input::is_key_pressed(KEY_ESCAPE))
            running = false;
    }

    bool TestStage::is_running()
    {
        return running;
    }
};
