#include "config.hpp"
#include "core/game.hpp"
#include "core/input.hpp"
#include "ecs/ecs.hpp"
#include "renderer/font.hpp"
#include "renderer/height_map.hpp"
#include "renderer/model.hpp"
#include "renderer/post/post_processing.hpp"
#include "renderer/primitives/box.hpp"
#include "renderer/primitives/line.hpp"
#include "renderer/primitives/sphere.hpp"
#include "renderer/shadow_map.hpp"
#include "renderer/skybox.hpp"
#include "tools/profiler.hpp"

namespace bls
{
    void render_scene(ECS &ecs, Shader &shader, Renderer &renderer);
    void render_colliders(ECS &ecs, const mat4 &projection, const mat4 &view);
    void render_texts(ECS &ecs);

    void render_system(ECS &ecs, f32 dt)
    {
        BLS_PROFILE_SCOPE("render_system");

        auto &window = Game::get().get_window();
        auto &renderer = Game::get().get_renderer();

        auto width = window.get_width();
        auto height = window.get_height();

        auto camera = ecs.cameras.begin()->second.get();
        auto position = camera->position;
        auto projection = camera->projection_matrix;
        auto view = camera->view_matrix;
        auto near = camera->near;
        auto far = camera->far;

        auto &shaders = renderer.get_shaders();
        auto &textures = renderer.get_textures();
        auto &g_buffer = renderer.get_gbuffer();
        auto &skybox = renderer.get_skybox();
        auto &quad = renderer.get_rendering_quad();
        auto &shadow_map = renderer.get_shadow_map();
        auto &height_map = renderer.get_height_map();
        auto &post_processing = renderer.get_post_processing();

        // Shaders - by now they should have been initialized
        auto g_buffer_shader = shaders["g_buffer"].get();
        auto pbr_shader = shaders["pbr"].get();
        auto ui_shader = shaders["ui"].get();

        // Reset the viewport
        renderer.clear_color({0.0f, 0.0f, 0.0f, 1.0f});
        renderer.clear();
        renderer.set_viewport(0, 0, width, height);

        // Render shadow map
        shadow_map->bind();
        render_scene(ecs, shadow_map->get_shadow_depth_shader(), renderer);
        shadow_map->unbind();

        // Reset the viewport
        renderer.clear_color({0.0f, 0.0f, 0.0f, 1.0f});
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

        // Render height map
        height_map->render(view, projection);

        // Render particles
        particle_system(ecs, dt);

        g_buffer->unbind();
        g_buffer_shader->unbind();

        // Lighting pass: calculate lighting using the gbuffer content
        // -------------------------------------------------------------------------------------------------------------
        renderer.clear();

        pbr_shader->bind();

        // Set camera position
        pbr_shader->set_uniform3("viewPos", position);

        // Set light uniforms
        pbr_shader->set_uniform3("lightDir", shadow_map->get_light_dir());
        pbr_shader->set_uniform1("near", near);
        pbr_shader->set_uniform1("far", far);

        // Set lights uniforms
        u32 light_counter = 0;

        // Point lights
        auto &point_lights = ecs.point_lights;
        auto &transforms = ecs.transforms;
        for (auto &[id, light] : point_lights)
        {
            auto transform = transforms[id].get();

            pbr_shader->set_uniform3("lights.pointLightPositions[" + to_str(light_counter) + "]", transform->position);
            pbr_shader->set_uniform3("lights.pointLightColors[" + to_str(light_counter) + "]", light->diffuse);

            light_counter++;
        }

        // Directional lights
        light_counter = 0;
        auto &dir_lights = ecs.dir_lights;
        for (auto &[id, light] : dir_lights)
        {
            auto transform = transforms[id].get();

            pbr_shader->set_uniform3("lights.dirLightDirections[" + to_str(light_counter) + "]", transform->rotation);
            pbr_shader->set_uniform3("lights.dirLightColors[" + to_str(light_counter) + "]", light->diffuse);

            light_counter++;
        }

        // Set texture attachments ---
        u32 tex_position = 0;
        std::shared_ptr<bls::Texture> ui_texture;
        for (const auto &[name, texture] : textures)
        {
            if (name == "ui")
            {
                ui_texture = texture;
                continue;
            }

            pbr_shader->set_uniform1("textures." + name, tex_position);
            texture->bind(tex_position);
            tex_position++;
        }

        // Bind maps
        skybox->bind(*pbr_shader, 12);           // IBL maps
        shadow_map->bind_maps(*pbr_shader, 15);  // Shadow map

        // Begin post processing process
        post_processing->begin();
        quad->render();  // Render light quad
        post_processing->end();

        // Copy content of geometry's depth buffer to default framebuffer's depth buffer
        // -------------------------------------------------------------------------------------------------------------
        g_buffer->bind_and_blit(width, height);
        g_buffer->unbind();

        // Draw the skybox last
        skybox->draw(view, projection);

        // Draw UI
        {
            auto tex_width = ui_texture->get_width();
            auto tex_height = ui_texture->get_height();

            renderer.set_viewport(
                (width / 2) - (tex_width / 4), (height / 2) - (tex_height / 4), tex_width / 2, tex_height / 2);

            ui_shader->bind();
            ui_shader->set_uniform1("screenTexture", 0U);
            ui_texture->bind(0);
            quad->render();
        }

// Render debug lines
#if !defined(_RELEASE)
        if (AppConfig::render_colliders) render_colliders(ecs, projection, view);
#endif

        // Render texts
        render_texts(ecs);
    }

    void render_scene(ECS &ecs, Shader &shader, Renderer &renderer)
    {
        // Render all entities
        for (const auto &[id, model] : ecs.models)
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

            // Player model matrix
            if (ecs.names[id] == "player")
            {
                // Rotate
                model_matrix = rotate(model_matrix, radians(transform->rotation.z), vec3(0.0f, 0.0f, 1.0f));
                model_matrix = rotate(model_matrix, radians(-transform->rotation.y + 90.0f), vec3(0.0f, 1.0f, 0.0f));
                model_matrix = rotate(model_matrix, radians(-transform->rotation.x), vec3(1.0f, 0.0f, 0.0f));
            }

            // Player bullet model matrix
            else if (ecs.names[id] == "bullet" && ecs.projectiles[id]->sender_id == 0)
            {
                model_matrix = rotate(model_matrix, radians(transform->rotation.z), vec3(0.0f, 0.0f, 1.0f));
                model_matrix = rotate(model_matrix, radians(-transform->rotation.y + 90.0f), vec3(0.0f, 1.0f, 0.0f));
                model_matrix = rotate(model_matrix, radians(-transform->rotation.x), vec3(1.0f, 0.0f, 0.0f));
            }

            // Ophanim model matrix
            else if (ecs.names[id] == "ophanim")
            {
                // Compensate for model rotation
                model_matrix = rotate(model_matrix, radians(transform->rotation.x), vec3(1.0f, 0.0f, 0.0f));
                model_matrix = rotate(model_matrix, radians(transform->rotation.y - 90.0f), vec3(0.0f, 1.0f, 0.0f));
                model_matrix = rotate(model_matrix, radians(transform->rotation.z), vec3(0.0f, 0.0f, 1.0f));
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
            for (const auto &mesh : model->model->meshes)
            {
                // Bind textures
                for (u32 i = 0; i < mesh->textures.size(); i++)
                {
                    auto texture = mesh->textures[i];

                    str type_name;
                    auto type = texture->get_type();

                    switch (type)
                    {
                        case TextureType::Diffuse:
                            type_name = "diffuse";
                            break;
                        case TextureType::Specular:
                            type_name = "specular";
                            break;
                        case TextureType::Normal:
                            type_name = "normal";
                            break;
                        case TextureType::Metalness:
                            type_name = "metalness";
                            break;
                        case TextureType::Roughness:
                            type_name = "roughness";
                            break;
                        case TextureType::AmbientOcclusion:
                            type_name = "ao";
                            break;
                        case TextureType::Emissive:
                            type_name = "emissive";
                            break;

                        default:
                            LOG_ERROR("invalid texture type");
                            break;
                    }

                    shader.set_uniform1("material." + type_name, i);
                    texture->bind(i);  // Offset the active samplers in the frag shader
                }

                mesh->vao->bind();
                renderer.draw_indexed(RenderingMode::Triangles, mesh->indices.size());

                // Reset
                mesh->vao->unbind();

                // Update stats
                AppStats::vertices += mesh->vertices.size();
            }
        }
    }

    void render_texts(ECS &ecs)
    {
        auto &texts = ecs.texts;
        auto &transforms = ecs.transforms;
        for (const auto &[id, text] : texts)
        {
            auto transform = transforms[id].get();
            text->font->render(
                text->text, transform->position.x, transform->position.y, transform->scale.x, text->color);
        }
    }

    void render_colliders(ECS &ecs, const mat4 &projection, const mat4 &view)
    {
        // Restore viewport
        auto &renderer = Game::get().get_renderer();
        auto &window = Game::get().get_window();
        renderer.set_viewport(0, 0, window.get_width(), window.get_height());

        // Set debug mode
        renderer.set_debug_mode(true);

        auto color_shader = Shader::create("color", "", "");
        color_shader->bind();

        color_shader->set_uniform4("projection", projection);
        color_shader->set_uniform4("view", view);
        color_shader->set_uniform4("model", mat4(1.0f));
        color_shader->set_uniform3("color", {1.0f, 0.0f, 0.0f});

        // Create axes lines for debugging              // Start    // End
        std::vector<std::unique_ptr<Line>> axes;
        axes.push_back(std::make_unique<Line>(renderer, vec3(0.0f), vec3(1000.0f, 0.0f, 0.0f)));  // x
        axes.push_back(std::make_unique<Line>(renderer, vec3(0.0f), vec3(0.0f, 1000.0f, 0.0f)));  // y
        axes.push_back(std::make_unique<Line>(renderer, vec3(0.0f), vec3(0.0f, 0.0f, 1000.0f)));  // z

        // Render axes lines
        for (u64 i = 0; i < axes.size(); i++)
        {
            vec3 color;

            if (i == 0) color = {1.0f, 0.0f, 0.0f};  // x axis is red
            if (i == 1) color = {0.0f, 1.0f, 0.0f};  // y axis is green
            if (i == 2) color = {0.0f, 0.0f, 1.0f};  // z axis is blue

            color_shader->set_uniform3("color", color);
            axes[i]->render();
        }

        // Render colliders
        for (const auto &[id, collider] : ecs.colliders)
        {
            auto &transform = ecs.transforms[id];

            color_shader->set_uniform3("color", collider->color);
            if (collider->type == Collider::ColliderType::Sphere)
            {
                auto collider_sphere = std::make_unique<Sphere>(renderer,
                                                                transform->position + collider->offset,
                                                                static_cast<SphereCollider *>(collider.get())->radius);

                collider_sphere->render();
            }

            else if (collider->type == Collider::ColliderType::Box)
            {
                auto collider_box = std::make_unique<Box>(renderer,
                                                          transform->position + collider->offset,
                                                          static_cast<BoxCollider *>(collider.get())->dimensions);

                collider_box->render();
            }

            // Render orientation vector
            auto pitch = transform->rotation.x;
            auto yaw = transform->rotation.y;
            auto front = vec3(
                cos(radians(yaw)) * cos(radians(pitch)), sin(radians(pitch)), sin(radians(yaw)) * cos(radians(pitch)));

            auto orientation_line =
                std::make_unique<Line>(renderer, transform->position, transform->position + (front * 30.0f));

            color_shader->set_uniform3("color", {1.0f, 0.0f, 1.0f});
            orientation_line->render();
        }

        // Unset debug mode
        color_shader->unbind();
        renderer.set_debug_mode(false);
    }
};  // namespace bls
