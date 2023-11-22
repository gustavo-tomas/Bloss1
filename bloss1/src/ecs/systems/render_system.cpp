#include "config.hpp"
#include "core/game.hpp"
#include "ecs/ecs.hpp"
#include "renderer/font.hpp"
#include "renderer/model.hpp"
#include "renderer/primitives/box.hpp"
#include "renderer/primitives/line.hpp"
#include "renderer/primitives/sphere.hpp"

namespace bls
{
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
                mesh->vao->unbind();

                // Update stats
                AppStats::vertices += mesh->vertices.size();
            }
        }
    }

    void render_ui()
    {
        auto &renderer = Game::get().get_renderer();
        auto &textures = renderer.get_textures();
        auto &shader = renderer.get_shaders()["ui"];
        auto &quad = renderer.get_rendering_quad();
        auto &window = Game::get().get_window();

        auto width = window.get_width();
        auto height = window.get_height();

        auto p = std::find_if(textures.begin(), textures.end(), [](auto p) { return p.first == "ui"; });
        std::shared_ptr<bls::Texture> ui_texture = p->second;

        auto tex_width = ui_texture->get_width();
        auto tex_height = ui_texture->get_height();

        renderer.set_viewport(
            (width / 2) - (tex_width / 4), (height / 2) - (tex_height / 4), tex_width / 2, tex_height / 2);

        shader->bind();
        shader->set_uniform1("screenTexture", 0U);
        ui_texture->bind(0);
        quad->render();
    }

    void render_texts(ECS &ecs)
    {
        auto &renderer = Game::get().get_renderer();
        auto &window = Game::get().get_window();

        auto width = window.get_width();
        auto height = window.get_height();

        renderer.set_viewport(0, 0, width, height);

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
