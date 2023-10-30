#pragma once

/**
 * @brief Simple file parser to extract scene data from a bloss file
 */

#include "core/core.hpp"
#include "ecs/ecs.hpp"

namespace bls
{
    class SceneParser
    {
        public:
            static void parse_scene(ECS &ecs, const str &file);
            static void save_scene(ECS &ecs, const str &file);
            static void save_config(const str &file);

        private:
            static void parse_component(ECS &ecs, const str &line, u32 entity_id, const str &entity_name);
            static void parse_config(const str &line, const str &config_name);
            static vec3 read_vec3(std::istringstream *iline, char delimiter);
            static vec2 read_vec2(std::istringstream *iline, char delimiter);
            static void write_vec3(std::ofstream *scene, const vec3 &vec, const str &end_str);
            static void write_vec2(std::ofstream *scene, const vec2 &vec, const str &end_str);
    };
};  // namespace bls
