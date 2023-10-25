#pragma once

/**
 * @brief Interface for the renderer. The platform must implement the renderer accordingly.
 */

#include "math/math.hpp"

namespace bls
{
    enum class RenderingMode
    {
        Lines, Triangles, TriangleStrip
    };

    class Skybox;
    class Quad;
    class ShadowMap;
    class PostProcessingSystem;
    class Shader;
    class Texture;
    class FrameBuffer;
    class ECS;

    // Renderer backend (OpenGL, Vulkan, Metal, DirectX, ...)
    class Renderer
    {
        public:
            virtual ~Renderer() { }

            virtual void initialize() = 0;

            virtual void set_viewport(u32 x, u32 y, u32 width, u32 height) = 0;
            virtual void set_debug_mode(bool active) = 0;
            virtual void set_blending(bool active) = 0;
            virtual void set_face_culling(bool active) = 0;

            virtual void clear_color(const vec4& color) = 0;
            virtual void clear() = 0;
            virtual void draw_indexed(RenderingMode mode, u32 count) = 0;
            virtual void draw_arrays(RenderingMode mode, u32 count) = 0;

            virtual void create_shadow_map(ECS& ecs) = 0;
            virtual void create_post_processing_passes() = 0;

            virtual std::map<str, std::shared_ptr<Shader>>& get_shaders() = 0;
            virtual std::vector<std::pair<str, std::shared_ptr<Texture>>>& get_textures() = 0;
            virtual std::unique_ptr<FrameBuffer>& get_gbuffer() = 0;
            virtual std::unique_ptr<Skybox>& get_skybox() = 0;
            virtual std::unique_ptr<Quad>& get_rendering_quad() = 0;
            virtual std::unique_ptr<ShadowMap>& get_shadow_map() = 0;
            virtual std::unique_ptr<PostProcessingSystem>& get_post_processing() = 0;

            // Must be implemented by the platform
            static Renderer* create();
    };
};
