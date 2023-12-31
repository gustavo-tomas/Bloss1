#include "renderer/opengl/renderer.hpp"

#include "platform/glfw/window.hpp"
#include "renderer/opengl/buffers.hpp"
#include "renderer/post/post_processing.hpp"
#include "renderer/primitives/quad.hpp"
#include "renderer/shadow_map.hpp"
#include "renderer/skybox.hpp"

namespace bls
{
    static GLenum convert_to_opengl_rendering_mode(RenderingMode mode)
    {
        switch (mode)
        {
            case RenderingMode::Lines:
                return GL_LINES;
            case RenderingMode::Triangles:
                return GL_TRIANGLES;
            case RenderingMode::TriangleStrip:
                return GL_TRIANGLE_STRIP;
            case RenderingMode::Patches:
                return GL_PATCHES;
            default:
                throw std::runtime_error("invalid rendering mode\n");
        }

        return 0;
    }

    OpenGLRenderer::~OpenGLRenderer()
    {
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

        // Gamma correction (done by the shaders)
        glDisable(GL_FRAMEBUFFER_SRGB);

        // Enable blending
        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Multisampling (MSAA)
        // glEnable(GL_MULTISAMPLE);

        auto &window = Game::get().get_window();
        auto width = window.get_width();
        auto height = window.get_height();

        // Create shaders
        // -------------------------------------------------------------------------------------------------------------

        // Geometry buffer shader
        shaders["g_buffer"] =
            Shader::create("g_buffer", "bloss1/assets/shaders/g_buffer.vs", "bloss1/assets/shaders/g_buffer.fs");

        // PBR shader
        shaders["pbr"] = Shader::create("pbr", "bloss1/assets/shaders/pbr/pbr.vs", "bloss1/assets/shaders/pbr/pbr.fs");

        // PBR shader
        shaders["f_pbr"] = Shader::create(
            "f_pbr", "bloss1/assets/shaders/pbr/pbr_forward.vs", "bloss1/assets/shaders/pbr/pbr_forward.fs");

        // Debug shader
        shaders["color"] = Shader::create(
            "color", "bloss1/assets/shaders/test/base_color.vs", "bloss1/assets/shaders/test/base_color.fs");

        // UI shader
        shaders["ui"] =
            Shader::create("ui", "bloss1/assets/shaders/post/base.vs", "bloss1/assets/shaders/post/base.fs");

        // Create g_buffer framebuffer
        g_buffer = std::unique_ptr<FrameBuffer>(FrameBuffer::create());

        // Create and attach framebuffer textures
        std::vector<str> texture_names = {"position", "normal", "albedo", "arm", "emissive", "depth"};
        for (const auto &name : texture_names)
        {
            auto texture = Texture::create(width,
                                           height,
                                           ImageFormat::RGBA32F,
                                           TextureParameter::Repeat,
                                           TextureParameter::Repeat,
                                           TextureParameter::Nearest,
                                           TextureParameter::Nearest);

            textures.push_back({name, texture});
            g_buffer->attach_texture(texture.get());
        }
        g_buffer->draw();

        // Create UI Texture
        auto texture = Texture::create("ui_texture", "bloss1/assets/textures/crosshair.png", TextureType::Diffuse);
        textures.push_back({"ui", texture});

        // Create and attach depth buffer
        render_buffer = std::unique_ptr<RenderBuffer>(RenderBuffer::create(width, height, AttachmentType::Depth));
        render_buffer->bind();

        // Check if framebuffer is complete
        if (!g_buffer->check()) throw std::runtime_error("framebuffer is not complete");
        g_buffer->unbind();

        // Create a quad for rendering
        quad = std::make_unique<Quad>(*this);

        // Create post processing system
        post_processing = std::make_unique<PostProcessingSystem>(width, height);
    }

    void OpenGLRenderer::set_viewport(u32 x, u32 y, u32 width, u32 height)
    {
        glViewport(x, y, width, height);
    }

    void OpenGLRenderer::set_debug_mode(bool active)
    {
        // Wireframe mode
        if (active)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_CULL_FACE);
        }

        // Normal mode
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_CULL_FACE);
        }
    }

    void OpenGLRenderer::set_blending(bool active)
    {
        if (active)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        else
            glDisable(GL_BLEND);
    }

    void OpenGLRenderer::set_face_culling(bool active)
    {
        if (active)
            glEnable(GL_CULL_FACE);

        else
            glDisable(GL_CULL_FACE);
    }

    void OpenGLRenderer::set_tesselation_patches(u32 patches)
    {
        glPatchParameteri(GL_PATCH_VERTICES, patches);
    }

    void OpenGLRenderer::clear_color(const vec4 &color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void OpenGLRenderer::clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void OpenGLRenderer::draw_indexed(RenderingMode mode, u32 count, const void *indices)
    {
        auto opengl_mode = convert_to_opengl_rendering_mode(mode);
        glDrawElements(opengl_mode, count, GL_UNSIGNED_INT, indices);
    }

    void OpenGLRenderer::draw_arrays(RenderingMode mode, u32 count)
    {
        auto opengl_mode = convert_to_opengl_rendering_mode(mode);
        glDrawArrays(opengl_mode, 0, count);
    }

    void OpenGLRenderer::create_skybox(const str &file,
                                       const u32 skybox_resolution,
                                       const u32 irradiance_resolution,
                                       const u32 brdf_resolution,
                                       const u32 prefilter_resolution,
                                       const u32 max_mip_levels)
    {
        skybox.reset();
        skybox = std::unique_ptr<Skybox>(Skybox::create(
            file, skybox_resolution, irradiance_resolution, brdf_resolution, prefilter_resolution, max_mip_levels));
    }

    void OpenGLRenderer::create_shadow_map(ECS &ecs)
    {
        // Create shadow map
        for (const auto &[id, dir_light] : ecs.dir_lights)
        {
            const auto &transform = ecs.transforms[id];
            auto dir = transform->rotation;
            dir.y *= -1.0f;
            shadow_map = std::make_unique<ShadowMap>(*ecs.cameras[0], normalize(dir));
        }
    }

    void OpenGLRenderer::create_height_map(
        u32 width, u32 height, u32 min_tess_level, u32 max_tess_level, f32 min_distance, f32 max_distance)
    {
        height_map =
            std::make_unique<HeightMap>(width, height, min_tess_level, max_tess_level, min_distance, max_distance);
    }

    void OpenGLRenderer::create_post_processing_passes(ECS &ecs)
    {
        auto &window = Game::get().get_window();
        auto &camera = ecs.cameras[0];

        auto width = window.get_width();
        auto height = window.get_height();

        u32 pass_position = 1;
        post_processing->add_pass(new FXAAPass(width, height), pass_position++);
        post_processing->add_pass(new BloomPass(width, height, 5, 7.0f, 0.4f, 0.325f), pass_position++);

        post_processing->add_pass(new FogPass(width,
                                              height,
                                              vec3(0.0f),
                                              vec2(camera->far / 3.0f, camera->far / 2.0f),
                                              camera->position,
                                              textures[0].second.get()),
                                  pass_position++);

        post_processing->add_pass(new SharpenPass(width, height, 0.05f), pass_position++);
        post_processing->add_pass(new PosterizationPass(width, height, 8.0f), pass_position++);
        post_processing->add_pass(new PixelizationPass(width, height, 4), pass_position++);
        post_processing->add_pass(new OutlinePass(width, height, vec3(0.0f), 0.8f), pass_position++);
        post_processing->add_pass(new VignettePass(width, height, 0.85f, 0.45f), pass_position++);
        post_processing->add_pass(new KuwaharaPass(width, height, 5), pass_position++);
    }

    std::map<str, std::shared_ptr<Shader>> &OpenGLRenderer::get_shaders()
    {
        return shaders;
    }

    std::vector<std::pair<str, std::shared_ptr<Texture>>> &OpenGLRenderer::get_textures()
    {
        return textures;
    }

    std::unique_ptr<FrameBuffer> &OpenGLRenderer::get_gbuffer()
    {
        return g_buffer;
    }

    std::unique_ptr<Skybox> &OpenGLRenderer::get_skybox()
    {
        return skybox;
    }

    std::unique_ptr<Quad> &OpenGLRenderer::get_rendering_quad()
    {
        return quad;
    }

    std::unique_ptr<ShadowMap> &OpenGLRenderer::get_shadow_map()
    {
        return shadow_map;
    }

    std::unique_ptr<HeightMap> &OpenGLRenderer::get_height_map()
    {
        return height_map;
    }

    std::unique_ptr<PostProcessingSystem> &OpenGLRenderer::get_post_processing()
    {
        return post_processing;
    }
};  // namespace bls
