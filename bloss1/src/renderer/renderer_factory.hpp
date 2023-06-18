#pragma once

/**
 * @brief The renderer factory creates a renderer with the chosen backend. For now,
 * only OpenGL is supported.
 */

#include "renderer/renderer.hpp"
#include "renderer/opengl/renderer.hpp"

namespace bls
{
    enum class BackendType
    {
        OpenGL = 0,
        // Vulkan = 1,
        // Metal = 2
        // Add more backend types as needed
    };

    class RendererFactory
    {
        public:
            static Renderer* create_renderer(BackendType backend)
            {
                switch (backend)
                {
                    // Add cases for other backend types
                    case BackendType::OpenGL:
                        return new OpenGLRenderer();
                        break;

                    default:
                        return nullptr; // Oops
                        break;
                }
            }
    };
};
