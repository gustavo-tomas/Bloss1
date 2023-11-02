#include "ecs/systems/particle_system.hpp"

#include "core/game.hpp"
#include "ecs/ecs.hpp"
#include "renderer/model.hpp"
#include "renderer/primitives/quad.hpp"
#include "renderer/shader.hpp"
#include "renderer/texture.hpp"
#include "tools/profiler.hpp"

namespace bls
{
    void particle_system(ECS &ecs, f32 dt)
    {
        // Emit particles
        for (auto &[id, particle_sys] : ecs.particle_systems)
        {
            while (particle_sys->particles_to_be_emitted > 0)
            {
                particle_sys->emitter->emit();
                particle_sys->particles_to_be_emitted--;
            }

            // Render particle
            particle_sys->emitter->render_particle(ecs, dt);
        }
    }

    // Emitter
    // -----------------------------------------------------------------------------------------------------------------
    Emitter::Emitter(const vec3 &center, EmitterType type, bool particle_2D) : type(type)
    {
        auto &renderer = Game::get().get_renderer();

        particle_shader = Shader::create("particle",
                                         "bloss1/assets/shaders/particles/particle.vs",
                                         "bloss1/assets/shaders/particles/particle_texture.fs");
        quad = std::make_unique<Quad>(renderer);
        pool_index = 999;
        particle_pool.resize(pool_index + 1);
        particle_texture =
            Texture::create("particle", "bloss1/assets/textures/particles/particle_black.png", TextureType::Diffuse);
        model = Model::create("particle", "bloss1/assets/models/particles/particle.obj", false);

        particle_to_emit.color_begin = {0.9f, 0.9f, 0.3f, 1.0f};
        particle_to_emit.color_end = {0.3f, 0.9f, 0.9f, 1.0f};
        particle_to_emit.scale_begin = vec3(0.5f);
        particle_to_emit.scale_variation = vec3(0.3f);
        particle_to_emit.scale_end = vec3(0.01f);
        particle_to_emit.life_time = 10.0f;
        particle_to_emit.velocity = vec3(0.0f);
        particle_to_emit.velocity_variation = {0.5f, 2.0f, 0.5f};
        particle_to_emit.position = center;

        this->center = center;
        this->particle_2D = particle_2D;
    }

    void Emitter::render_particle(ECS &ecs, f32 dt)
    {
        BLS_PROFILE_SCOPE("particle_system");

        auto &renderer = Game::get().get_renderer();
        renderer.set_blending(true);
        renderer.set_face_culling(false);

        auto camera = ecs.cameras.begin()->second.get();

        for (auto &particle : particle_pool)
        {
            if (!particle.active) continue;

            if (particle.life_remaining <= 0.0f)
            {
                particle.active = false;
                continue;
            }

            particle.life_remaining -= dt;
            particle.position += particle.velocity * dt;
            particle.rotation += 0.01f * dt;
        }

        particle_shader->bind();
        particle_shader->set_uniform1("particleTexture", 0U);
        particle_shader->set_uniform4("projection", camera->projection_matrix);
        particle_shader->set_uniform4("view", camera->view_matrix);

        for (const auto &particle : particle_pool)
        {
            if (!particle.active) continue;

            // Fade away particles
            f32 life = particle.life_remaining / particle.life_time;
            vec4 color = mix(particle.color_end, particle.color_begin, life);
            vec3 size = mix(particle.scale_end, particle.scale_begin, life);

            // Render
            mat4 model_matrix = translate(mat4(1.0f), particle.position) *
                                rotate(mat4(1.0f), particle.rotation.z, {0.0f, 0.0f, 1.0f}) *
                                rotate(mat4(1.0f), particle.rotation.y, {0.0f, 1.0f, 0.0f}) *
                                rotate(mat4(1.0f), particle.rotation.x, {1.0f, 0.0f, 0.0f}) * scale(mat4(1.0f), size);

            particle_shader->set_uniform4("model", model_matrix);
            particle_shader->set_uniform4("color", color);

            // 2D particle (texture)
            if (particle_2D)
            {
                particle_texture->bind(0);
                quad->render();
            }

            // 3D particle (model)
            else
            {
                for (auto &mesh : model->meshes)
                {
                    mesh->vao->bind();
                    renderer.draw_indexed(RenderingMode::Triangles, mesh->indices.size());
                    mesh->vao->unbind();

                    AppStats::vertices += mesh->vertices.size();
                }
            }
        }

        renderer.set_face_culling(true);
        renderer.set_blending(false);
    }

    void Emitter::emit_particle(const Particle &particle_props)
    {
        auto &random_engine = Game::get().get_random_engine();

        Particle &particle = particle_pool[pool_index];
        particle.active = true;
        particle.position = particle_props.position;
        particle.rotation.z = random_engine.get_float(0.0f, 2.0f * glm::pi<f32>());

        // Velocity
        particle.velocity = particle_props.velocity;
        for (u16 i = 0; i < 3; i++)
            particle.velocity[i] += particle_props.velocity_variation[i] * (random_engine.get_float());

        particle.scale_begin =
            particle_props.scale_begin + particle_props.scale_variation * (random_engine.get_float());
        particle.scale_end = particle_props.scale_end;

        // Color
        particle.color_begin = particle_props.color_begin;
        particle.color_end = particle_props.color_end;

        particle.life_time = particle_props.life_time;
        particle.life_remaining = particle_props.life_time;

        pool_index = min(--pool_index, static_cast<u32>(particle_pool.size() - 1));
    }

    void Emitter::set_center(const vec3 &new_center)
    {
        this->center = new_center;
    }

    void Emitter::set_particle(const Particle &particle)
    {
        this->particle_to_emit = particle;
    }

    Particle Emitter::get_particle()
    {
        return particle_to_emit;
    }

    // SphereEmitter
    // -----------------------------------------------------------------------------------------------------------------
    SphereEmitter::SphereEmitter(const vec3 &center, bool particle_2D, f32 radius)
        : Emitter(center, EmitterType::Sphere, particle_2D)
    {
        this->center = center;
        this->radius = radius;
    }

    void SphereEmitter::emit()
    {
        Particle particle = particle_to_emit;
        particle.position = generate_random_point_on_surface();

        emit_particle(particle);
    }

    vec3 SphereEmitter::generate_random_point_on_surface()
    {
        auto random_engine = Game::get().get_random_engine();

        f32 theta = random_engine.get_float(0.0f, 2.0f * M_PI);  // Azimuthal angle
        f32 phi = random_engine.get_float(0.0f, M_PI);           // Polar angle

        f32 x = center.x + radius * sin(phi) * cos(theta);
        f32 y = center.y + radius * sin(phi) * sin(theta);
        f32 z = center.z + radius * cos(phi);

        return vec3(x, y, z);
    }

    // BoxEmitter
    // -----------------------------------------------------------------------------------------------------------------
    BoxEmitter::BoxEmitter(const vec3 &center, bool particle_2D, const vec3 &dimensions)
        : Emitter(center, EmitterType::Box, particle_2D)
    {
        this->center = center;
        this->dimensions = dimensions;
    }

    void BoxEmitter::emit()
    {
        Particle particle = particle_to_emit;
        particle.position = generate_random_point_on_surface();

        emit_particle(particle);
    }

    vec3 BoxEmitter::generate_random_point_on_surface()
    {
        auto random_engine = Game::get().get_random_engine();

        vec3 min_aabb = center - dimensions;
        vec3 max_aabb = center + dimensions;

        vec3 point;
        for (u32 i = 0; i < 3; i++) point[i] = random_engine.get_float(min_aabb[i], max_aabb[i]);

        u32 face = random_engine.get_int(0, 6);
        switch (face)
        {
            case 0:
                point.x = max_aabb.x;
                break;
            case 1:
                point.x = min_aabb.x;
                break;
            case 2:
                point.y = max_aabb.y;
                break;
            case 3:
                point.y = min_aabb.y;
                break;
            case 4:
                point.z = max_aabb.z;
                break;
            case 5:
                point.z = min_aabb.z;
                break;
        }

        return point;
    }
};  // namespace bls
