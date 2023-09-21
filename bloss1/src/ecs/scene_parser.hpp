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
            SceneParser(ECS& ecs);
            ~SceneParser();

            void parse_scene(const str& file);
            void save_scene(const str& file);

        private:
            void parse_component(const str& line, u32 entity_id, const str& entity_name);
            vec3 read_vec3(std::istringstream* iline, char delimiter);

            ECS& ecs;
    };
};
