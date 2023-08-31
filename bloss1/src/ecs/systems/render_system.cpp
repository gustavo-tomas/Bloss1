#include "ecs/ecs.hpp"
#include "renderer/model.hpp"
#include "renderer/shader.hpp"
#include "renderer/primitives/box.hpp"
#include "renderer/primitives/line.hpp"
#include "renderer/primitives/sphere.hpp"
#include "core/game.hpp"

namespace bls
{
    void render_system(ECS& ecs, f32 dt)
    {
        // Shaders - by now they should have been initialized
        auto g_buffer_shader = Shader::create("g_buffer", "", "");

        // Render all entities
        auto& renderer = Game::get().get_renderer();
        for (const auto& [id, model] : ecs.models)
        {
            // Reset bone matrices
            for (u32 i = 0; i < MAX_BONE_MATRICES; i++)
                g_buffer_shader->set_uniform4("finalBonesMatrices[" + to_str(i) + "]", mat4(1.0f));

            // Update animators
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

                // Update bone matrices
                auto bone_matrices = animator->get_final_bone_matrices();
                for (u32 i = 0; i < bone_matrices.size(); i++)
                    g_buffer_shader->set_uniform4("finalBonesMatrices[" + to_str(i) + "]", bone_matrices[i]);
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
                renderer.draw_indexed(RenderingMode::Triangles, mesh->indices.size());

                // Reset
                mesh->vao->unbind();
            }
        }

        // Render debug lines
        // render_colliders(ecs, projection, view);
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
                                       ecs.transforms[id]->position,
                                       static_cast<SphereCollider*>(collider.get())->radius);

                collider_sphere->render();
            }

            else if (collider->type == Collider::ColliderType::Box)
            {
                auto collider_box = std::make_unique<Box>(renderer, ecs.transforms[id]->position,
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
