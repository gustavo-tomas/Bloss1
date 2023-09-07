#include "ecs/ecs.hpp"
#include "core/game.hpp"
#include "renderer/model.hpp"
#include "renderer/shader.hpp"
#include "renderer/skybox.hpp"
#include "renderer/font.hpp"
#include "renderer/shadow_map.hpp"
#include "renderer/primitives/box.hpp"
#include "renderer/primitives/line.hpp"
#include "renderer/primitives/quad.hpp"
#include "renderer/primitives/sphere.hpp"

namespace bls
{
    void render_scene(ECS& ecs, Shader& shader, Renderer& renderer);
    void render_colliders(ECS& ecs, const mat4& projection, const mat4& view);

    // @TODO: oooooooooooffffff
    static bool initialized = false;

    struct RenderState
    {
        std::unique_ptr<Quad> quad;
        std::unique_ptr<FrameBuffer> g_buffer;
        std::unique_ptr<RenderBuffer> render_buffer;
        std::map<str, std::shared_ptr<Shader>> shaders;

        std::map<str, std::shared_ptr<Font>> fonts;
        std::unordered_map<str, std::shared_ptr<Texture>> textures;

        Skybox* skybox;
        std::unique_ptr<ShadowMap> shadow_map;
    };

    RenderState render_state;

    void initialize(ECS& ecs)
    {
        render_state = { };

        auto& renderer = Game::get().get_renderer();
        auto& window = Game::get().get_window();

        auto width = window.get_width();
        auto height = window.get_height();

        // Create shaders
        // -------------------------------------------------------------------------------------------------------------

        // Geometry buffer shader
        render_state.shaders["g_buffer"] = Shader::create("g_buffer", "bloss1/assets/shaders/g_buffer.vs", "bloss1/assets/shaders/g_buffer.fs");

        // PBR shader
        render_state.shaders["pbr"] = Shader::create("pbr", "bloss1/assets/shaders/pbr/pbr.vs", "bloss1/assets/shaders/pbr/pbr.fs");

        // Debug shader
        render_state.shaders["color"] = Shader::create("color", "bloss1/assets/shaders/test/base_color.vs", "bloss1/assets/shaders/test/base_color.fs");

        // Create g_buffer framebuffer
        render_state.g_buffer = std::unique_ptr<FrameBuffer>(FrameBuffer::create());

        // Create and attach framebuffer textures
        // @TODO: this is hardcoded
        std::vector<str> texture_names = { "position", "normal", "albedo", "arm", "tbnNormal", "depth" };
        for (const auto& name : texture_names)
        {
            render_state.textures[name] = Texture::create(width, height, ImageFormat::RGBA32F,
                                          TextureParameter::Repeat, TextureParameter::Repeat,
                                          TextureParameter::Nearest, TextureParameter::Nearest);

            render_state.g_buffer->attach_texture(render_state.textures[name].get());
        }
        render_state.g_buffer->draw();

        // Create and attach depth buffer
        render_state.render_buffer = std::unique_ptr<RenderBuffer>(RenderBuffer::create(width, height, AttachmentType::Depth));
        render_state.render_buffer->bind();

        // Check if framebuffer is complete
        if (!render_state.g_buffer->check())
            exit(1);

        render_state.g_buffer->unbind();

        // Create a skybox
        // skybox = Skybox::create("bloss1/assets/textures/newport_loft.hdr", 1024, 32, 2048, 2048, 12);
        render_state.skybox = Skybox::create("bloss1/assets/textures/pine_attic_4k.hdr", 1024, 32, 1024, 1024, 10);
        // render_state.skybox = Skybox::create("bloss1/assets/textures/moonlit_golf_4k.hdr", 512, 32, 512, 512, 10);

        // Create a quad for rendering
        render_state.quad = std::make_unique<Quad>(renderer);

        // Create font
        render_state.fonts["inder"] = Font::create("inder_regular", "bloss1/assets/font/inder_regular.ttf");
        render_state.fonts["lena"]  = Font::create("lena", "bloss1/assets/font/lena.ttf");

        // Create shadow map
        for (const auto& [id, dir_light] : ecs.dir_lights)
        {
            const auto& transform = ecs.transforms[id];
            auto dir = transform->rotation;
            dir.y *= -1.0f;
            render_state.shadow_map = std::make_unique<ShadowMap>(*ecs.cameras[0].get(), normalize(dir));
        }
    }

    void render_system(ECS& ecs, f32 dt)
    {
        if (!initialized)
        {
            initialize(ecs);
            initialized = true;
        }

        auto& window = Game::get().get_window();
        auto& renderer = Game::get().get_renderer();

        auto width = window.get_width();
        auto height = window.get_height();

        // @TODO: player id
        auto camera = ecs.cameras[0].get();
        auto position = camera->position;
        auto projection = camera->projection_matrix;
        auto view = camera->view_matrix;
        auto near = camera->near;
        auto far = camera->far;

        auto& shaders = render_state.shaders;
        auto& textures = render_state.textures;
        auto& g_buffer = render_state.g_buffer;
        auto& skybox = render_state.skybox;
        auto& quad = render_state.quad;
        auto& fonts = render_state.fonts;
        auto& shadow_map = render_state.shadow_map;

        // Shaders - by now they should have been initialized
        auto g_buffer_shader = shaders["g_buffer"].get();
        auto pbr_shader = shaders["pbr"].get();

        // Reset the viewport
        renderer.clear_color({ 0.0f, 0.0f, 0.0f, 1.0f });
        renderer.clear();
        renderer.set_viewport(0, 0, width, height);

        // Update animators
        for (auto& [id, model] : ecs.models)
        {
            auto animator = model->model->animator.get();
            if (animator)
            {
                // @TODO: test - Blend animations
                if (ecs.names[id] == "abomination")
                {
                    auto& animations = model->model->animations;
                    auto twist_anim = animations["Armature|Twist"].get();
                    auto rotate_anim = animations["Armature|ArmatureAction.005"].get();
                    f32 blend_factor = 0.5f;

                    animator->blend_animations(rotate_anim, twist_anim, blend_factor, dt);
                }

                else
                    animator->update(dt);
            }
        }

        // @TODO: see if this can be done during geometry pass
        // Render shadow map
        shadow_map->bind();
        render_scene(ecs, shadow_map->get_shadow_depth_shader(), renderer);
        shadow_map->unbind();

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

        // Render the scene
        render_scene(ecs, *g_buffer_shader, renderer);

        g_buffer->unbind();
        g_buffer_shader->unbind();

        // Lighting pass: calculate lighting using the gbuffer content
        // -------------------------------------------------------------------------------------------------------------
        renderer.clear();

        pbr_shader->bind();

        // Set camera position
        pbr_shader->set_uniform3("viewPos", position);

        // Set fog uniforms
        pbr_shader->set_uniform3("fogColor", vec3(0.5f));
        pbr_shader->set_uniform2("fogMinMax", vec2(far / 3.0f, far));

        // Set light uniforms
        pbr_shader->set_uniform3("lightDir", render_state.shadow_map->get_light_dir());
        pbr_shader->set_uniform1("near", near);
        pbr_shader->set_uniform1("far", far);

        // Set lights uniforms
        u32 light_counter = 0;

        // Point lights
        auto& point_lights = ecs.point_lights;
        auto& transforms = ecs.transforms;
        for (auto& [id, light] : point_lights)
        {
            auto transform = transforms[id].get();

            pbr_shader->set_uniform3("lights.pointLightPositions[" + to_str(light_counter) + "]", transform->position);
            pbr_shader->set_uniform3("lights.pointLightColors[" + to_str(light_counter) + "]", light->diffuse);

            light_counter++;
        }

        // Directional lights
        light_counter = 0;
        auto& dir_lights = ecs.dir_lights;
        for (auto& [id, light] : dir_lights)
        {
            auto transform = transforms[id].get();

            pbr_shader->set_uniform3("lights.dirLightDirections[" + to_str(light_counter) + "]", transform->rotation);
            pbr_shader->set_uniform3("lights.dirLightColors[" + to_str(light_counter) + "]", light->diffuse);

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

        // Bind maps
        skybox->bind(*pbr_shader, 10);          // IBL maps
        shadow_map->bind_maps(*pbr_shader, 13); // Shadow map

        // Render light quad
        quad->render();

        // Copy content of geometry's depth buffer to default framebuffer's depth buffer
        // -----------------------------------------------------------------------------------------------------------------
        g_buffer->bind_and_blit(width, height);
        g_buffer->unbind();

        // Draw the skybox last
        skybox->draw(view, projection);

        // Render text
        fonts["inder"]->render("owowowowow", 20.0f, 20.0f, 0.5f, { 0.95f, 0.6f, 0.4f });
        fonts["lena"]->render("lalala", 600.0f, 400.0f, 0.75f, { 0.4f, 0.6f, 0.8f });

        // Render debug lines
        render_colliders(ecs, projection, view);
    }

    void render_scene(ECS& ecs, Shader& shader, Renderer& renderer)
    {
        // Render all entities
        for (const auto& [id, model] : ecs.models)
        {
            // Reset bone matrices
            for (u32 i = 0; i < MAX_BONE_MATRICES; i++)
                shader.set_uniform4("finalBonesMatrices[" + to_str(i) + "]", mat4(1.0f));

            // Update animators
            auto animator = model->model->animator.get();
            if (animator)
            {
                // Update bone matrices
                auto bone_matrices = animator->get_final_bone_matrices();
                for (u32 i = 0; i < bone_matrices.size(); i++)
                    shader.set_uniform4("finalBonesMatrices[" + to_str(i) + "]", bone_matrices[i]);
            }

            // Remember: scale -> rotate -> translate
            auto transform = ecs.transforms[id].get();
            auto model_matrix = mat4(1.0f);

            // Translate
            model_matrix = translate(model_matrix, transform->position);

            // @TODO: i dont know what im doing but it works

            // Player model matrix
            if (ecs.names[id] == "player")
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
            shader.set_uniform4("model", model_matrix);

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

                    shader.set_uniform1("material." + type_name, i);
                    texture->bind(i); // Offset the active samplers in the frag shader
                }

                mesh->vao->bind();
                renderer.draw_indexed(RenderingMode::Triangles, mesh->indices.size());

                // Reset
                mesh->vao->unbind();
            }
        }
    }

    void render_colliders(ECS& ecs, const mat4& projection, const mat4& view)
    {
        // Set debug mode
        auto& renderer = Game::get().get_renderer();
        renderer.set_debug_mode(true);

        auto color_shader = Shader::create("color", "", "");
        color_shader->bind();

        color_shader->set_uniform4("projection", projection);
        color_shader->set_uniform4("view", view);
        color_shader->set_uniform4("model", mat4(1.0f));
        color_shader->set_uniform3("color", { 1.0f, 0.0f, 0.0f });

        // Create axes lines for debugging              // Start    // End
        std::vector<std::unique_ptr<Line>> axes;
        axes.push_back(std::make_unique<Line>(renderer, vec3(0.0f), vec3(1000.0f, 0.0f, 0.0f))); // x
        axes.push_back(std::make_unique<Line>(renderer, vec3(0.0f), vec3(0.0f, 1000.0f, 0.0f))); // y
        axes.push_back(std::make_unique<Line>(renderer, vec3(0.0f), vec3(0.0f, 0.0f, 1000.0f))); // z

        // Render axes lines
        for (u64 i = 0; i < axes.size(); i++)
        {
            vec3 color;

            if (i == 0) color = { 1.0f, 0.0f, 0.0f }; // x axis is red
            if (i == 1) color = { 0.0f, 1.0f, 0.0f }; // y axis is green
            if (i == 2) color = { 0.0f, 0.0f, 1.0f }; // z axis is blue

            color_shader->set_uniform3("color", color);
            axes[i]->render();
        }

        // Render colliders
        for (const auto& [id, collider] : ecs.colliders)
        {
            color_shader->set_uniform3("color", collider->color);
            if (collider->type == Collider::ColliderType::Sphere)
            {
                auto collider_sphere = std::make_unique<Sphere>(renderer,
                                       ecs.transforms[id]->position + collider->offset,
                                       static_cast<SphereCollider*>(collider.get())->radius);

                collider_sphere->render();
            }

            else if (collider->type == Collider::ColliderType::Box)
            {
                auto collider_box = std::make_unique<Box>(renderer, ecs.transforms[id]->position + collider->offset,
                                    static_cast<BoxCollider*>(collider.get())->width,
                                    static_cast<BoxCollider*>(collider.get())->height,
                                    static_cast<BoxCollider*>(collider.get())->depth);

                collider_box->render();
            }
        }

        // Unset debug mode
        color_shader->unbind();
        renderer.set_debug_mode(false);
    }
};
