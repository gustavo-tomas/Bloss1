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
        delete skybox;
    }

    void TestStage::start()
    {
        // Create the ECS
        ecs = std::unique_ptr<ECS>(new ECS());

        // Add systems in order of execution
        ecs->add_system(transform_system);
        ecs->add_system(render_system);

        // Add some entities to the world
        for (u32 i = 0; i < 5; i++)
            player(*ecs, Transform(vec3(i * 10.0f, 5.0f, 5.0f), vec3(0.0f), vec3(5.0f)));

        floor(*ecs, Transform(vec3(0.0f), vec3(0.0f), vec3(10.0f, 1.0f, 10.0f)));

        // Create a camera and controller
        vec3 offset = vec3(15.0f, 7.0f, 50.0f);
        controller = new CameraController(ecs->transforms[0]->position, ecs->transforms[0]->rotation, offset);

        // Add directional lights
        dir_light_id = directional_light(*ecs,
                                         Transform(vec3(0.0f), vec3(0.3f, -1.0f, 0.15f)),
                                         DirectionalLight(vec3(0.2f), vec3(1.0f), vec3(1.0f))
                                        );

        // Add point lights
        point_light(*ecs, Transform(vec3( 100.0f, 100.0f,  100.0f)), PointLight(vec3(40000.0f)));
        point_light(*ecs, Transform(vec3( 100.0f, 100.0f, -100.0f)), PointLight(vec3(40000.0f)));
        point_light(*ecs, Transform(vec3(-100.0f, 100.0f,  100.0f)), PointLight(vec3(40000.0f)));
        point_light(*ecs, Transform(vec3(-100.0f, 100.0f, -100.0f)), PointLight(vec3(40000.0f)));

        // Create shaders

        // Geometry buffer shader
        g_buffer_shader = Shader::create("g_buffer", "bloss1/assets/shaders/g_buffer.vs", "bloss1/assets/shaders/g_buffer.fs");

        // PBR shader
        pbr_shader = Shader::create("pbr", "bloss1/assets/shaders/pbr/pbr.vs", "bloss1/assets/shaders/pbr/pbr.fs");

        // Create framebuffer textures
        g_buffer = std::unique_ptr<FrameBuffer>(FrameBuffer::create());

        // Position
        position_texture = Texture::create(window.get_width(), window.get_height(), ImageFormat::RGBA32F,
                                           TextureParameter::Repeat, TextureParameter::Repeat,
                                           TextureParameter::Nearest, TextureParameter::Nearest);
        g_buffer->attach_texture(position_texture.get());

        // Normal
        normal_texture = Texture::create(window.get_width(), window.get_height(), ImageFormat::RGBA32F,
                                         TextureParameter::Repeat, TextureParameter::Repeat,
                                         TextureParameter::Nearest, TextureParameter::Nearest);
        g_buffer->attach_texture(normal_texture.get());

        // Albedo
        albedo_texture = Texture::create(window.get_width(), window.get_height(), ImageFormat::RGBA8,
                                         TextureParameter::Repeat, TextureParameter::Repeat,
                                         TextureParameter::Nearest, TextureParameter::Nearest);
        g_buffer->attach_texture(albedo_texture.get());

        // ARM
        arm_texture = Texture::create(window.get_width(), window.get_height(), ImageFormat::RGBA8,
                                      TextureParameter::Repeat, TextureParameter::Repeat,
                                      TextureParameter::Nearest, TextureParameter::Nearest);
        g_buffer->attach_texture(arm_texture.get());

        // TBN normal
        tbn_texture = Texture::create(window.get_width(), window.get_height(), ImageFormat::RGBA32F,
                                      TextureParameter::Repeat, TextureParameter::Repeat,
                                      TextureParameter::Nearest, TextureParameter::Nearest);
        g_buffer->attach_texture(tbn_texture.get());

        // Depth
        depth_texture = Texture::create(window.get_width(), window.get_height(), ImageFormat::RGB32F,
                                        TextureParameter::Repeat, TextureParameter::Repeat,
                                        TextureParameter::Nearest, TextureParameter::Nearest);
        g_buffer->attach_texture(depth_texture.get());

        g_buffer->draw();

        // Create and attach depth buffer
        render_buffer = std::unique_ptr<RenderBuffer>(RenderBuffer::create(window.get_width(), window.get_height(), AttachmentType::Depth));
        render_buffer->bind();

        // Check if framebuffer is complete
        if (!g_buffer->check())
            exit(1);

        g_buffer->unbind();

        // Create a skybox
        // skybox = Skybox::create("bloss1/assets/textures/newport_loft.hdr", 1024, 32, 2048, 2048, 12);
        skybox = Skybox::create("bloss1/assets/textures/pine_attic_4k.hdr", 1024, 32, 1024, 1024, 10);

        // Create a quad for rendering
        quad = std::make_unique<Quad>(renderer);

        // Create font
        inder_font = Font::create("inder_regular", "bloss1/assets/font/inder_regular.ttf");
        lena_font  = Font::create("lena", "bloss1/assets/font/lena.ttf");

        // Create a video player
        video_player = std::make_unique<VideoPlayer>("bloss1/assets/videos/mh_pro_skate.mp4");

        // Play the video
        video_player->play_video();

        running = true;
    }

    void TestStage::update(f32 dt)
    {
        // Update camera controller
        controller->update(dt);
        controller->get_camera().update(dt);

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
            // Remember: scale -> rotate -> translate
            auto transform = ecs->transforms[id].get();
            auto model_matrix = mat4(1.0f);

            // Translate
            model_matrix = translate(model_matrix, transform->position);

            // @TODO: i dont know what im doing but it works

            // Player model matrix
            if (id == 0)
            {
                // Rotate
                model_matrix = rotate(model_matrix, radians(transform->rotation.z), vec3(0.0f, 0.0f, 1.0f));
                model_matrix = rotate(model_matrix, radians(-transform->rotation.y - 90.0f), vec3(0.0f, 1.0f, 0.0f));
                model_matrix = rotate(model_matrix, radians(transform->rotation.x), vec3(1.0f, 0.0f, 0.0f));
            }

            else
            {
                // Rotate
                model_matrix = rotate(model_matrix, radians(transform->rotation.x), vec3(1.0f, 0.0f, 0.0f));
                model_matrix = rotate(model_matrix, radians(transform->rotation.y), vec3(0.0f, 1.0f, 0.0f));
                model_matrix = rotate(model_matrix, radians(transform->rotation.z), vec3(0.0f, 0.0f, 1.0f));
            }

            // Scale
            model_matrix = scale(model_matrix, transform->scale);

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

        // Set camera position
        pbr_shader->set_uniform3("viewPos", position);

        // Set lights uniforms
        u32 light_counter = 0;
        auto& point_lights = ecs->point_lights;
        auto& light_transforms = ecs->transforms;
        for (auto& [id, light] : point_lights)
        {
            auto transform = light_transforms[id].get();

            pbr_shader->set_uniform3("pointLightPositions[" + to_str(light_counter) + "]", transform->position);
            pbr_shader->set_uniform3("pointLightColors[" + to_str(light_counter) + "]", light->diffuse);

            light_counter++;
        }

        // @TODO: this is hardcoded
        std::vector<str> textures = { "position", "normal", "albedo", "arm", "tbnNormal", "depth" };
        auto attachments = g_buffer->get_attachments();
        for (u32 i = 0; i < attachments.size(); i++)
        {
            pbr_shader->set_uniform1("textures." + textures[i], i);
            attachments[i]->bind(i);
        }

        // Bind IBL maps
        skybox->bind(*pbr_shader.get(), 10);

        // Render light quad
        quad->render();

        // Copy content of geometry's depth buffer to default framebuffer's depth buffer
        // -----------------------------------------------------------------------------------------------------------------
        g_buffer->bind_and_blit(width, height);
        g_buffer->unbind();

        // Draw the skybox last
        skybox->draw(view, projection);

        // Render text
        inder_font->render("owowowowow", 20.0f, 20.0f, 0.5f, { 0.95f, 0.6f, 0.4f });
        lena_font->render("lalala", 600.0f, 400.0f, 0.75f, { 0.4f, 0.6f, 0.8f });

        // Exit the stage
        if (Input::is_key_pressed(KEY_ESCAPE))
            running = false;
    }

    bool TestStage::is_running()
    {
        return running;
    }
};
