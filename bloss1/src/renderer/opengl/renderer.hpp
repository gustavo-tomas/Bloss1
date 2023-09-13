#pragma once

/**
 * @brief The renderer implementation for OpenGL.
 */

#include "renderer/renderer.hpp"

namespace bls
{
    class OpenGLRenderer : public Renderer
    {
        public:
            ~OpenGLRenderer();

            void initialize() override;

            void set_viewport(u32 x, u32 y, u32 width, u32 height) override;
            void set_debug_mode(bool active) override;
            void set_blending(bool active) override;
            void set_face_culling(bool active) override;

            void clear_color(const vec4& color) override;
            void clear() override;
            void draw_indexed(RenderingMode mode, u32 count) override;
            void draw_arrays(RenderingMode mode, u32 count) override;
    };
};
