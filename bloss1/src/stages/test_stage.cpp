#include "stages/test_stage.hpp"
#include "core/input.hpp"
#include "ecs/systems.hpp"
#include "ecs/entities.hpp"

namespace bls
{
    TestStage::TestStage(Renderer& renderer, Window& window, AudioEngine& audio_engine)
        : renderer(renderer), window(window), audio_engine(audio_engine)
    {

    }

    TestStage::~TestStage()
    {
        delete controller;
        delete skybox;
    }

    void TestStage::start()
    {
        // Load audios
        // audio_engine.load("test", "bloss1/assets/sounds/toc.wav", false);
        // audio_engine.set_echo_filter("test", 0.2f, 0.15f);
        // audio_engine.play("test");

        // Create the ECS
        ecs = std::unique_ptr<ECS>(new ECS());

        // Add systems in order of execution
        ecs->add_system(animation_system);
        ecs->add_system(physics_system);
        ecs->add_system(render_system);

        // Add some entities to the world
        for (u32 i = 0; i < 5; i++)
            player(*ecs, Transform(vec3(i * 10.0f, 30.0f, 0.0f), vec3(0.0f, 90.0f, 0.0f), vec3(5.0f)));

        vampire(*ecs, Transform(vec3(-20.0f, 40.0f, 0.0f), vec3(0.0f), vec3(0.001f, 0.001f, 0.001f)));
        abomination(*ecs, Transform(vec3(-30.0f, 40.0f, 0.0f), vec3(-90.0f, 0.0f, 180.0f), vec3(1.0f, 1.0f, 1.0f))); // @TODO: fix rotation

        // Floor is created last
        floor(*ecs, Transform(vec3(0.0f), vec3(0.0f), vec3(10.0f, 1.0f, 10.0f)));

        // Create a camera and controller
        vec3 offset = vec3(15.0f, 7.0f, 50.0f);
        controller = new CameraController(ecs->transforms[0]->position, ecs->transforms[0]->rotation, offset, *ecs->physics_objects[0].get());

        // Add directional lights
        directional_light(*ecs,
                          Transform(vec3(0.0f), vec3(0.3f, -1.0f, 0.15f)),
                          DirectionalLight(vec3(0.2f), vec3(1.0f), vec3(1.0f)));

        // Add point lights
        point_light(*ecs, Transform(vec3( 100.0f, 100.0f,  100.0f)), PointLight(vec3(40000.0f)));
        point_light(*ecs, Transform(vec3( 100.0f, 100.0f, -100.0f)), PointLight(vec3(40000.0f)));
        point_light(*ecs, Transform(vec3(-100.0f, 100.0f,  100.0f)), PointLight(vec3(40000.0f)));
        point_light(*ecs, Transform(vec3(-100.0f, 100.0f, -100.0f)), PointLight(vec3(40000.0f)));

        // Create shaders

        // Geometry buffer shader
        shaders["g_buffer"] = Shader::create("g_buffer", "bloss1/assets/shaders/g_buffer.vs", "bloss1/assets/shaders/g_buffer.fs");

        // PBR shader
        shaders["pbr"] = Shader::create("pbr", "bloss1/assets/shaders/pbr/pbr.vs", "bloss1/assets/shaders/pbr/pbr.fs");

        // Debug shader
        shaders["color"] = Shader::create("color", "bloss1/assets/shaders/test/base_color.vs", "bloss1/assets/shaders/test/base_color.fs");

        // Create g_buffer framebuffer
        g_buffer = std::unique_ptr<FrameBuffer>(FrameBuffer::create());

        // Create and attach framebuffer textures
        // @TODO: this is hardcoded
        std::vector<str> texture_names = { "position", "normal", "albedo", "arm", "tbnNormal", "depth" };
        for (const auto& name : texture_names)
        {
            textures[name] = Texture::create(window.get_width(), window.get_height(), ImageFormat::RGBA32F,
                                             TextureParameter::Repeat, TextureParameter::Repeat,
                                             TextureParameter::Nearest, TextureParameter::Nearest);

            g_buffer->attach_texture(textures[name].get());
        }
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
        // video_player = std::make_unique<VideoPlayer>("bloss1/assets/videos/mh_pro_skate.mp4");

        // // Play the video
        // video_player->play_video();

        running = true;
    }

    void TestStage::update(f32 dt)
    {
        // Update camera controller
        controller->update(dt);
        controller->get_camera().update(dt);

        // Window properties
        auto width = window.get_width();
        auto height = window.get_height();

        // Camera properties
        auto projection = controller->get_camera().get_projection_matrix(width, height);
        auto view = controller->get_camera().get_view_matrix();
        auto position = controller->get_camera().get_position();

        // Shaders
        auto g_buffer_shader = shaders["g_buffer"].get();
        auto pbr_shader = shaders["pbr"].get();

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

        // Update all systems in registration order
        auto& systems = ecs->systems;
        for (auto& system : systems)
            system(*ecs, dt);

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

        // Set texture attachments
        auto& attachments = g_buffer->get_attachments();
        u32 tex_position = attachments.size() - 1;
        for (const auto& [name, texture] : textures)
        {
            pbr_shader->set_uniform1("textures." + name, tex_position);
            attachments[tex_position]->bind(tex_position);
            tex_position--; // Traverse from back to front
        }

        // Bind IBL maps
        skybox->bind(*pbr_shader, 10);

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

        // Render debug lines
        // render_colliders(projection, view);

        // Exit the stage
        if (Input::is_key_pressed(KEY_ESCAPE))
            running = false;
    }

    bool TestStage::is_running()
    {
        return running;
    }
};
