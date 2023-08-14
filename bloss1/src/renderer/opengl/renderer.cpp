#include "renderer/opengl/renderer.hpp"
#include "renderer/opengl/buffers.hpp"
#include "platform/glfw/window.hpp"

namespace bls
{
    static GLenum convert_to_opengl_rendering_mode(RenderingMode mode)
    {
        switch (mode)
        {
            case RenderingMode::Lines:  return GL_LINES;
            case RenderingMode::Triangles: return GL_TRIANGLES;
            case RenderingMode::TriangleStrip: return GL_TRIANGLE_STRIP;
            default: std::cerr << "invalid rendering mode: '" << mode << "'\n"; exit(1);
        }

        return 0;
    }

    OpenGLRenderer::~OpenGLRenderer()
    {
        std::cout << "opengl renderer destroyed successfully\n";
    }

    void OpenGLRenderer::initialize()
    {
        // Enable depth testing
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // Enable stencil testing
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        // Cull triangles which normal is not towards the camera
        // glEnable(GL_CULL_FACE); // @TODO: investigate skybox

        // Remove cubemap seams
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        // Enable blending
        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Gamma correction (done by the shaders)
        glDisable(GL_FRAMEBUFFER_SRGB);

        // Multisampling (MSAA)
        // glEnable(GL_MULTISAMPLE);
    }

    void OpenGLRenderer::set_viewport(u32 x, u32 y, u32 width, u32 height)
    {
        glViewport(x, y, width, height);
    }

    void OpenGLRenderer::set_debug_mode(bool active)
    {
        // Wireframe mode
        if (active)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // Normal mode
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    void OpenGLRenderer::clear_color(const vec4& color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void OpenGLRenderer::clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void OpenGLRenderer::draw_indexed(RenderingMode mode, u32 count)
    {
        auto opengl_mode = convert_to_opengl_rendering_mode(mode);
        glDrawElements(opengl_mode, count, GL_UNSIGNED_INT, 0);
    }

    void OpenGLRenderer::draw_arrays(RenderingMode mode, u32 count)
    {
        auto opengl_mode = convert_to_opengl_rendering_mode(mode);
        glDrawArrays(opengl_mode, 0, count);
    }
};
