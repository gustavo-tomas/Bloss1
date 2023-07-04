#include "renderer/opengl/skybox.hpp"

#include "core/game.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace bls
{
    OpenGLSkybox::OpenGLSkybox(const str& path, u32 dimensions)
    {
        // @TODO: maybe make configurable
        const u32 max_mip_levels = 5;
        const u32 prefilter_resolution = 128;
        const u32 brdf_resolution = 512;

        // Shaders
        hdr_to_cubemap_shader = Shader::create("hdr_to_cubemap", "bloss1/assets/shaders/pbr/hdr_cubemap_converter.vs", "bloss1/assets/shaders/pbr/hdr_cubemap_converter.fs");
        skybox_shader = Shader::create("skybox", "bloss1/assets/shaders/pbr/skybox.vs", "bloss1/assets/shaders/pbr/skybox.fs");
        irradiance_shader = Shader::create("irradiance", "bloss1/assets/shaders/pbr/irradiance_map.vs", "bloss1/assets/shaders/pbr/irradiance_map.fs");
        prefilter_shader = Shader::create("prefilter", "bloss1/assets/shaders/pbr/prefilter.vs", "bloss1/assets/shaders/pbr/prefilter.fs");
        brdf_shader = Shader::create("brdf", "bloss1/assets/shaders/pbr/brdf.vs", "bloss1/assets/shaders/pbr/brdf.fs");

        skybox_shader->bind();
        skybox_shader->set_uniform1("environmentMap", 0U);

        // Cube setup
        cube = new Cube(Game::get().get_renderer());
        quad = new Quad(Game::get().get_renderer());

        // Setup framebuffers
        // -------------------------------------------------------------------------------------------------------------
        captureFBO = FrameBuffer::create();
        captureRBO = RenderBuffer::create(dimensions, dimensions, AttachmentType::Depth);

        hdr_texture = Texture::create(path, path, TextureType::None);

        // Setup cubemap to render to and attach to framebuffer
        // -------------------------------------------------------------------------------------------------------------
        glGenTextures(1, &env_cubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap);
        for (u32 i = 0; i < 6; i++)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, dimensions, dimensions, 0, GL_RGB, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Setup projection and view matrices for capturing data onto the 6 cubemap face directions
        // -------------------------------------------------------------------------------------------------------------
        const mat4 captureProjection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);
        const mat4 captureViews[] =
        {
            lookAt(vec3(0.0f, 0.0f, 0.0f), vec3( 1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
            lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
            lookAt(vec3(0.0f, 0.0f, 0.0f), vec3( 0.0f,  1.0f,  0.0f), vec3(0.0f,  0.0f,  1.0f)),
            lookAt(vec3(0.0f, 0.0f, 0.0f), vec3( 0.0f, -1.0f,  0.0f), vec3(0.0f,  0.0f, -1.0f)),
            lookAt(vec3(0.0f, 0.0f, 0.0f), vec3( 0.0f,  0.0f,  1.0f), vec3(0.0f, -1.0f,  0.0f)),
            lookAt(vec3(0.0f, 0.0f, 0.0f), vec3( 0.0f,  0.0f, -1.0f), vec3(0.0f, -1.0f,  0.0f))
        };

        // Convert HDR equirectangular environment map to cubemap equivalent
        // -------------------------------------------------------------------------------------------------------------
        hdr_to_cubemap_shader->bind();
        hdr_to_cubemap_shader->set_uniform1("equirectangularMap", 0U);
        hdr_to_cubemap_shader->set_uniform4("projection", captureProjection);

        glBindTextureUnit(0, hdr_texture->get_id());

        glViewport(0, 0, dimensions, dimensions); // don't forget to configure the viewport to the capture dimensions.
        captureFBO->bind();
        for (u32 i = 0; i < 6; i++)
        {
            hdr_to_cubemap_shader->set_uniform4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, env_cubemap, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            cube->render();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Create an irradiance cubemap
        // -------------------------------------------------------------------------------------------------------------
        glGenTextures(1, &irradiance_map);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_map);
        for (u32 i = 0; i < 6; i++)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        captureFBO->bind();
        captureRBO->bind();
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

        // Configure irradiance shader
        // -------------------------------------------------------------------------------------------------------------
        irradiance_shader->bind();
        irradiance_shader->set_uniform1("environmentMap", 0U);
        irradiance_shader->set_uniform4("projection", captureProjection);

        glBindTextureUnit(0, env_cubemap);

        glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
        captureFBO->bind();
        for (u32 i = 0; i < 6; i++)
        {
            irradiance_shader->set_uniform4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_map, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            cube->render();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Configure pre filtering map for specular reflections
        // -------------------------------------------------------------------------------------------------------------
        glGenTextures(1, &prefilter_map);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_map);

        for (u32 i = 0; i < 6; i++)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, prefilter_resolution, prefilter_resolution, 0, GL_RGB, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        // Pre-filter the environment map
        // -------------------------------------------------------------------------------------------------------------
        prefilter_shader->bind();
        prefilter_shader->set_uniform1("environmentMap", 0U);
        prefilter_shader->set_uniform4("projection", captureProjection);

        glBindTextureUnit(0, env_cubemap);

        captureFBO->bind();
        for (u32 mip = 0; mip < max_mip_levels; mip++)
        {
            // Resize framebuffer according to mip-level size
            u32 mipWidth  = prefilter_resolution * pow(0.5, mip);
            u32 mipHeight = prefilter_resolution * pow(0.5, mip);

            captureRBO->bind();
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
            glViewport(0, 0, mipWidth, mipHeight);

            f32 roughness = (f32) mip / (f32) (max_mip_levels - 1);
            prefilter_shader->set_uniform1("roughness", roughness);
            for (u32 i = 0; i < 6; i++)
            {
                prefilter_shader->set_uniform4("view", captureViews[i]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter_map, mip);

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                cube->render();
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Generate texture for the BRDF convolution
        // -------------------------------------------------------------------------------------------------------------
        glGenTextures(1, &brdf_texture);

        // Pre-allocate enough memory for the BRDF texture.
        glBindTexture(GL_TEXTURE_2D, brdf_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdf_resolution, brdf_resolution, 0, GL_RG, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        captureFBO->bind();
        captureRBO->bind();
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, brdf_resolution, brdf_resolution);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf_texture, 0);

        glViewport(0, 0, brdf_resolution, brdf_resolution);
        brdf_shader->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        quad->render();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Reset the viewport to the screen dimensions
        auto width = Game::get().get_window().get_width();
        auto height = Game::get().get_window().get_height();
        glViewport(0, 0, width, height);
    }

    OpenGLSkybox::~OpenGLSkybox()
    {
        delete cube;
        delete quad;

        delete captureFBO;
        delete captureRBO;

        glDeleteTextures(1, &env_cubemap);
        glDeleteTextures(1, &irradiance_map);
        glDeleteTextures(1, &brdf_texture);

        std::cout << "cubemap destroyed successfully\n";
    }

    void OpenGLSkybox::bind(Shader& shader, u32 slot)
    {
        shader.bind();

        // Bind pre-computed maps
        shader.set_uniform1("textures.irradianceMap", slot);
        glBindTextureUnit(slot, irradiance_map);

        shader.set_uniform1("textures.prefilterMap", slot + 1);
        glBindTextureUnit(slot + 1, prefilter_map);

        shader.set_uniform1("textures.brdfLut", slot + 2);
        glBindTextureUnit(slot + 2, brdf_texture);
    }

    void OpenGLSkybox::draw(const mat4& view, const mat4& projection)
    {
        glDepthFunc(GL_LEQUAL);

        // Render cubemap
        skybox_shader->bind();
        skybox_shader->set_uniform4("view", view);
        skybox_shader->set_uniform4("projection", projection);

        glBindTextureUnit(0, env_cubemap);

        cube->render();

        glDepthFunc(GL_LESS);
    }
};
