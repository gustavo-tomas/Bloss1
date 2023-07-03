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

        // Geometry buffer shader
        g_buffer_shader = Shader::create("g_buffer", "bloss1/assets/shaders/g_buffer.vs", "bloss1/assets/shaders/g_buffer.fs");

        // PBR shader
        pbr_shader = Shader::create("pbr", "bloss1/assets/shaders/pbr.vs", "bloss1/assets/shaders/pbr.fs");

        // Create framebuffer textures
        g_buffer = std::unique_ptr<FrameBuffer>(FrameBuffer::create());

        // Position
        position_texture = Texture::create(window.get_width(), window.get_height(), ImageFormat::RGBA32F);
        g_buffer->attach_texture(position_texture.get());

        // Normal
        normal_texture = Texture::create(window.get_width(), window.get_height(), ImageFormat::RGBA32F);
        g_buffer->attach_texture(normal_texture.get());

        // Albedo
        albedo_texture = Texture::create(window.get_width(), window.get_height(), ImageFormat::RGBA8);
        g_buffer->attach_texture(albedo_texture.get());

        // ARM
        arm_texture = Texture::create(window.get_width(), window.get_height(), ImageFormat::RGBA8);
        g_buffer->attach_texture(arm_texture.get());

        // TBN normal
        tbn_texture = Texture::create(window.get_width(), window.get_height(), ImageFormat::RGBA32F);
        g_buffer->attach_texture(tbn_texture.get());

        // Depth
        depth_texture = Texture::create(window.get_width(), window.get_height(), ImageFormat::RGB32F);
        g_buffer->attach_texture(depth_texture.get());

        g_buffer->draw();

        // Create and attach depth buffer
        render_buffer = std::unique_ptr<RenderBuffer>(RenderBuffer::create(window.get_width(), window.get_height(), AttachmentType::Depth));
        render_buffer->bind();

        // Check if framebuffer is complete
        if (!g_buffer->check())
            exit(1);

        g_buffer->unbind();

        quad = std::make_unique<Quad>(renderer);

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

        // Reset the viewport
        renderer.clear_color({ 0.0f, 0.0f, 0.0f, 1.0f });
        renderer.clear();
        renderer.set_viewport(0, 0, width, height);

        // Geometry pass: render scene data into gbuffer
        // -------------------------------------------------------------------------------------------------------------
        g_buffer->bind();
        renderer.clear();

        g_buffer_shader->bind();
        g_buffer_shader->set_uniform4("projection", projection);
        g_buffer_shader->set_uniform4("view", view);

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
            g_buffer_shader->set_uniform4("model", model_matrix);

            // Render the model
            for (auto& mesh : model->model->meshes)
            {
                // Bind textures
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

                    g_buffer_shader->set_uniform1("material." + type_name, i);
                    texture->bind(i); // Offset the active samplers in the frag shader
                }

                mesh->vao->bind();
                renderer.draw_indexed(mesh->indices.size());

                // Reset
                mesh->vao->unbind();
            }
        }
        g_buffer->unbind();
        g_buffer_shader->unbind();

        // Lighting pass: calculate lighting using the gbuffer content
        // -------------------------------------------------------------------------------------------------------------
        renderer.clear();

        pbr_shader->bind();

        // @TODO: this is hardcoded
        std::vector<str> textures = { "position", "normal", "albedo", "arm", "tbnNormal", "depth" };
        auto attachments = g_buffer->get_attachments();
        for (u32 i = 0; i < attachments.size(); i++)
        {
            pbr_shader->set_uniform1("textures." + textures[i], i);
            attachments[i]->bind(i);
        }

        // Render light quad
        quad->Render();

        // Exit the stage
        if (Input::is_key_pressed(KEY_ESCAPE))
            running = false;
    }

    bool TestStage::is_running()
    {
        return running;
    }
};
