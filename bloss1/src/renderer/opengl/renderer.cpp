#include "renderer/opengl/renderer.hpp"
#include "renderer/opengl/buffers.hpp"
#include "platform/glfw/window.hpp"

namespace bls
{
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

        // Wireframe mode
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    void OpenGLRenderer::set_uniform()
    {

    }

    void OpenGLRenderer::set_texture()
    {

    }

    void OpenGLRenderer::set_render_target()
    {

    }

    void OpenGLRenderer::set_viewport(u32 x, u32 y, u32 width, u32 height)
    {
        glViewport(x, y, width, height);
    }

    void OpenGLRenderer::clear_color(const vec4& color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void OpenGLRenderer::clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void OpenGLRenderer::draw_indexed(u32 count)
    {
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
    }

    void OpenGLRenderer::draw_arrays(u32 count)
    {
        glDrawArrays(GL_TRIANGLES, 0, count);
    }

    void OpenGLRenderer::draw_lines(u32 count)
    {
        glDrawArrays(GL_LINES, 0, count);
    }
};
