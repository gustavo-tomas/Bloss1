#pragma once

/**
 * @brief @TODO:
 */

#include "ecs/ecs.hpp"
#include "renderer/shader.hpp"

namespace bls
{
    void render_scene(ECS &ecs, Shader &shader, Renderer &renderer);
    void render_colliders(ECS &ecs, const mat4 &projection, const mat4 &view);
    void render_texts(ECS &ecs);
};  // namespace bls
