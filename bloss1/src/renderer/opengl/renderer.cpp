#include "renderer/opengl/renderer.hpp"
#include "renderer/opengl/buffers.hpp"
#include "renderer/opengl/shader.hpp"
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
        glEnable(GL_CULL_FACE);

        // Remove cubemap seams
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        // Enable blending
        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Gamma correction
        // glEnable(GL_FRAMEBUFFER_SRGB);

        // Multisampling (MSAA)
        // glEnable(GL_MULTISAMPLE);

        // Wireframe mode
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    VertexBuffer* OpenGLRenderer::create_vertex_buffer(f32* vertices, u32 size)
    {
        return new OpenGLVertexBuffer(vertices, size);
    }

    IndexBuffer* OpenGLRenderer::create_index_buffer(u32* indices, u32 count)
    {
        return new OpenGLIndexBuffer(indices, count);
    }

    VertexArray* OpenGLRenderer::create_vertex_array()
    {
        return new OpenGLVertexArray();
    }

    Shader* OpenGLRenderer::create_shader(const str& vertex_path, const str& fragment_path, const str& geometry_path)
    {
        return new OpenGLShader(vertex_path, fragment_path, geometry_path);
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

    void OpenGLRenderer::draw()
    {
        // glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }
};
