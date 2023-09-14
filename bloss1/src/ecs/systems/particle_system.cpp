#include "ecs/systems/particle_system.hpp"
#include "ecs/ecs.hpp"
#include "core/game.hpp"
#include "renderer/shader.hpp"
#include "renderer/primitives/quad.hpp"
#include "renderer/texture.hpp"

namespace bls
{
    void initialize();

    // @TODO: aaaaaaaaaaaahhh
    static bool initialized = false;

    struct ParticleState
    {
        std::vector<Particle> particle_pool;
        u32 pool_index = 999;

        std::shared_ptr<Shader> particle_shader;
        std::unique_ptr<Quad> quad;
        std::shared_ptr<Texture> particle_texture;
    };

    ParticleState particle_state;

    void initialize()
    {
        auto& renderer = Game::get().get_renderer();

        particle_state.particle_shader = Shader::create("particle", "bloss1/assets/shaders/particles/particle.vs", "bloss1/assets/shaders/particles/particle_texture.fs");
        particle_state.quad = std::make_unique<Quad>(renderer);
        particle_state.particle_pool.resize(1000);
        particle_state.particle_texture = Texture::create("particle", "bloss1/assets/textures/particles/particle_black.png", TextureType::Diffuse);
    }

    void particle_system(ECS& ecs, f32 dt)
    {
        if (!initialized)
        {
            initialize();
            initialized = true;
        }

        auto& renderer = Game::get().get_renderer();
        renderer.set_blending(true);
        renderer.set_face_culling(false);

        // @TODO hardcoded
        auto camera = ecs.cameras[0].get();

        for (auto& particle : particle_state.particle_pool)
        {
            if (!particle.active)
                continue;

            if (particle.life_remaining <= 0.0f)
            {
                particle.active = false;
                continue;
            }

            particle.life_remaining -= dt;
            particle.position += particle.velocity * dt;
            particle.rotation += 0.01f * dt;
        }

        particle_state.particle_shader->bind();
        particle_state.particle_shader->set_uniform1("particleTexture", 0U);
        particle_state.particle_shader->set_uniform4("projection", camera->projection_matrix);
        particle_state.particle_shader->set_uniform4("view", camera->view_matrix);

        for (const auto& particle : particle_state.particle_pool)
        {
            if (!particle.active)
                continue;

            // Fade away particles
            f32 life = particle.life_remaining / particle.life_time;
            vec4 color = mix(particle.color_end, particle.color_begin, life);
            vec3 size = mix(particle.scale_end, particle.scale_begin, life);

            // Render @TODO: check rotation
            mat4 model_matrix = translate(mat4(1.0f), particle.position)
                                * rotate(mat4(1.0f), particle.rotation.z, { 0.0f, 0.0f, 1.0f })
                                * rotate(mat4(1.0f), particle.rotation.y, { 0.0f, 1.0f, 0.0f })
                                * rotate(mat4(1.0f), particle.rotation.x, { 1.0f, 0.0f, 0.0f })
                                * scale(mat4(1.0f), size);

            particle_state.particle_shader->set_uniform4("model", model_matrix);
            particle_state.particle_shader->set_uniform4("color", color);
            particle_state.particle_texture->bind(0);
            particle_state.quad->render();
        }

        renderer.set_face_culling(true);
        renderer.set_blending(false);
    }

    void emit_particle(const Particle& particle_props)
    {
        auto& random_engine = Game::get().get_random_engine();

        Particle& particle = particle_state.particle_pool[particle_state.pool_index];
        particle.active = true;
        particle.position = particle_props.position;
        particle.rotation.z = random_engine.get_float(0.0f, 2.0f * glm::pi<f32>());

        // Velocity
        particle.velocity = particle_props.velocity;
        for (u16 i = 0; i < 3; i++)
            particle.velocity[i] += particle_props.velocity_variation[i] * (random_engine.get_float());

        particle.scale_begin = particle_props.scale_begin + particle_props.scale_variation * (random_engine.get_float());
        particle.scale_end = particle_props.scale_end;

        // Color
        particle.color_begin = particle_props.color_begin;
        particle.color_end = particle_props.color_end;

        particle.life_time = particle_props.life_time;
        particle.life_remaining = particle_props.life_time;

        particle_state.pool_index = min(--particle_state.pool_index, static_cast<u32>(particle_state.particle_pool.size() - 1));
    }
};
