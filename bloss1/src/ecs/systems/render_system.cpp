#include "ecs/systems.hpp"
#include "managers/shader_manager.hpp"
#include "core/game.hpp"

namespace bls
{
    void render_system(ECS& ecs, f32 dt)
    {
        // Shaders
        auto g_buffer_shader = ShaderManager::get().get_shader("g_buffer");

        // Update animators
        for (const auto& [id, model] : ecs.models)
        {
            auto animator = model->model->animator.get();
            if (!animator)
                continue;

            animator->update(dt);

            // Update bone matrices
            auto bone_matrices = animator->get_final_bone_matrices();
            for (u32 i = 0; i < bone_matrices.size(); i++)
                g_buffer_shader->set_uniform4("finalBonesMatrices[" + to_str(i) + "]", bone_matrices[i]);
        }

        // Render all entities
        auto& renderer = Game::get().get_renderer();
        for (const auto& [id, model] : ecs.models)
        {
            // Remember: scale -> rotate -> translate
            auto transform = ecs.transforms[id].get();
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
                renderer.draw_indexed(RenderingMode::Triangles, mesh->indices.size());

                // Reset
                mesh->vao->unbind();
            }
        }
    }
};
