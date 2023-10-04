#pragma once

/**
 * @brief @TODO: this might not be a system suited for the ecs.
 *
 */

#include "ecs/ecs.hpp"

namespace bls
{
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
            virtual ~Emitter() { }

            virtual void emit() = 0;
    };

    class SphereEmitter : public Emitter
    {
        public:
            SphereEmitter(const vec3& center, f32 radius);

            void emit() override;
            void set_center(const vec3& new_center);

        private:
            vec3 generate_random_point_on_surface();

            vec3 center;
            f32 radius;
    };

    class BoxEmitter : public Emitter
    {
        public:
            BoxEmitter(const vec3& center, const vec3& dimensions);

            void emit() override;
            void set_center(const vec3& new_center);

        private:
            vec3 generate_random_point_on_surface();

            vec3 center;
            vec3 dimensions;
    };

    void particle_system(ECS& ecs, f32 dt);
    void emit_particle(const Particle& particle_props);
};
