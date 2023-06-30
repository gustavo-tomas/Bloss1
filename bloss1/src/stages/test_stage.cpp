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
        for (u32 i = 0; i < 5; i++)
            player(*ecs, Transform(vec3(i * 10.0f, 5.0f, 5.0f), vec3(0.0f), vec3(5.0f)));

        floor(*ecs, Transform(vec3(0.0f), vec3(0.0f), vec3(10.0f, 1.0f, 10.0f)));

        // Add lights
        dir_light_id = directional_light(*ecs,
                                         Transform(vec3(0.0f), vec3(0.3f, -1.0f, 0.15f)),
                                         DirectionalLight(vec3(0.2f), vec3(1.0f), vec3(1.0f)));

        point_light_id = point_light(*ecs,
                                     Transform(vec3(0.0f), vec3(0.3f, -1.0f, 0.15f)),
                                     PointLight(vec3(0.2f), vec3(1.0f), vec3(1.0f), 1.0f, 0.0001f, 0.000001f));

        // Create shaders
        phong_shader = Shader::create("test", "bloss1/assets/shaders/test/phong.vs", "bloss1/assets/shaders/test/phong.fs");
        phong_shader->bind();

        phong_shader->set_uniform3("dirLight.direction", ecs->transforms[dir_light_id]->rotation);
        phong_shader->set_uniform3("dirLight.ambient", ecs->dir_lights[dir_light_id]->ambient);
        phong_shader->set_uniform3("dirLight.diffuse", ecs->dir_lights[dir_light_id]->diffuse);
        phong_shader->set_uniform3("dirLight.specular", ecs->dir_lights[dir_light_id]->specular);

        phong_shader->set_uniform3("pointLight.position", ecs->transforms[point_light_id]->position);
        phong_shader->set_uniform3("pointLight.ambient", ecs->point_lights[point_light_id]->ambient);
        phong_shader->set_uniform3("pointLight.diffuse", ecs->point_lights[point_light_id]->diffuse);
        phong_shader->set_uniform3("pointLight.specular", ecs->point_lights[point_light_id]->specular);
        phong_shader->set_uniform1("pointLight.constant", ecs->point_lights[point_light_id]->constant);
        phong_shader->set_uniform1("pointLight.linear", ecs->point_lights[point_light_id]->linear);
        phong_shader->set_uniform1("pointLight.quadratic", ecs->point_lights[point_light_id]->quadratic);

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
        auto position = controller->get_camera().get_position();

        // Clear the screen
        renderer.clear();
        renderer.clear_color({ 0.4f, 0.6f, 0.8f, 1.0f });

        // Render all entities
        for (const auto& [id, model] : ecs->models)
        {
            auto transform = ecs->transforms[id].get();
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
            phong_shader->bind();
            phong_shader->set_uniform4("model", model_matrix);
            phong_shader->set_uniform4("projection", projection);
            phong_shader->set_uniform4("view", view);
            phong_shader->set_uniform3("viewPos", position);

            // Render the model
            for (auto& mesh : model->model->meshes)
            {
                // Bind textures
                i32 offset = 0; // @TODO: calculate precise offset
                for (u32 i = 0; i < mesh->textures.size(); i++)
                {
                    auto texture = mesh->textures[i];

                    str type_name;
                    auto type = texture->get_type();

                    switch (type)
                    {
                        case TextureType::Diffuse:          type_name = "diffuse";   break;
                        case TextureType::Specular:         type_name = "specular";  break;
                        case TextureType::Normal:           type_name = "normal";    break;
                        case TextureType::Metalness:        type_name = "metalness"; break;
                        case TextureType::Roughness:        type_name = "roughness"; break;
                        case TextureType::AmbientOcclusion: type_name = "ao";        break;

                        default: std::cerr << "invalid texture type: '" << type << "'\n";
                    }

                    phong_shader->set_uniform1("material." + type_name, i + offset);
                    texture->bind(i + offset); // Offset the active samplers in the frag shader
                }

                mesh->vao->bind();
                renderer.draw_indexed(mesh->indices.size());

                // Reset
                mesh->vao->unbind();
            }
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
