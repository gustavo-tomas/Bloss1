#include "ecs/systems.hpp"

namespace bls
{
    void render_system(ECS& ecs, f32 dt)
    {
        auto& models = ecs.models;
        for (auto& [id, model] : models)
        {
            // @TODO: for now, do nothing
        }
    }
};
