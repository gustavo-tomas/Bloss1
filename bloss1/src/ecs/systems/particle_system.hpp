#pragma once

/**
 * @brief Generic particle system. Can emit 2D texture or 3D objects on the surface of the specified shape.
 */

#include "core/core.hpp"
#include "math/math.hpp"
#include "renderer/model.hpp"
#include "renderer/primitives/quad.hpp"
#include "renderer/shader.hpp"
#include "renderer/texture.hpp"

namespace bls
{
    class ECS;
    void particle_system(ECS &ecs, f32 dt);

    struct Particle
    {
            vec3 position = vec3(0.0f);
            vec3 velocity = vec3(0.0f), velocity_variation = vec3(0.0f);
            vec4 color_begin = vec4(0.0f), color_end = vec4(0.0f);

            vec3 rotation = vec3(0.0f);
            vec3 scale_begin = vec3(1.0f), scale_end = vec3(0.0f), scale_variation = vec3(0.0f);

            f32 life_time = 1.0f;
            f32 life_remaining = 0.0f;

            bool active = false;
    };

    class Emitter
    {
        public:
            enum class EmitterType
            {
                Box,
                Sphere
            };

            Emitter(const vec3 &center, EmitterType type, bool particle_2D = false);
            virtual ~Emitter() = default;

            virtual void emit() = 0;
            virtual void render_particle(ECS &ecs, f32 dt);
            virtual void set_center(const vec3 &new_center);

            const EmitterType type;
            bool particle_2D;
            vec3 center;

        protected:
            virtual void emit_particle(const Particle &particle_props);

            std::vector<Particle> particle_pool;
            u32 pool_index;

            std::shared_ptr<Shader> particle_shader;
            std::unique_ptr<Quad> quad;
            std::shared_ptr<Texture> particle_texture;
            std::shared_ptr<Model> model;
    };

    class SphereEmitter : public Emitter
    {
        public:
            SphereEmitter(const vec3 &center, bool particle_2D, f32 radius);

            void emit() override;

            f32 radius;

        private:
            vec3 generate_random_point_on_surface();
    };

    class BoxEmitter : public Emitter
    {
        public:
            BoxEmitter(const vec3 &center, bool particle_2D, const vec3 &dimensions);

            void emit() override;

            vec3 dimensions;

        private:
            vec3 generate_random_point_on_surface();
    };
};  // namespace bls
