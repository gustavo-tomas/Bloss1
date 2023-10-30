#include "renderer/shadow_map.hpp"

#include <GL/glew.h>  // Include glew before glfw

#include "GLFW/glfw3.h"
#include "core/game.hpp"
#include "core/input.hpp"

namespace bls
{
    ShadowMap::ShadowMap(Camera &camera, const vec3 &light_dir) : camera(camera)
    {
        this->light_dir = light_dir;

        u32 zoom_factor = 10.0f;
        shadow_cascade_levels = {camera.far / (5.0f * zoom_factor),
                                 camera.far / (2.5f * zoom_factor),
                                 camera.far / (1.0f * zoom_factor),
                                 camera.far / (0.2f * zoom_factor)};
        depth_map_resolution = 4096;

        shadow_map_depth = Shader::create("shadow_map_depth",
                                          "bloss1/assets/shaders/shadow_map_depth.vs",
                                          "bloss1/assets/shaders/shadow_map_depth.fs",
                                          "bloss1/assets/shaders/shadow_map_depth.gs");

        debug_depth = Shader::create(
            "debug_depth", "bloss1/assets/shaders/test/debug_depth.vs", "bloss1/assets/shaders/test/debug_depth.fs");

        debug_depth->bind();
        debug_depth->set_uniform1("depthMap", 0U);

        debug_quad = std::make_unique<Quad>(Game::get().get_renderer());

        // Create light depth buffers
        glGenFramebuffers(1, &light_FBO);

        glGenTextures(1, &light_depth_maps);
        glBindTexture(GL_TEXTURE_2D_ARRAY, light_depth_maps);
        glTexImage3D(GL_TEXTURE_2D_ARRAY,
                     0,
                     GL_DEPTH_COMPONENT32F,
                     depth_map_resolution,
                     depth_map_resolution,
                     (int)shadow_cascade_levels.size() + 1,
                     0,
                     GL_DEPTH_COMPONENT,
                     GL_FLOAT,
                     nullptr);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        constexpr f32 bordercolor[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

        glBindFramebuffer(GL_FRAMEBUFFER, light_FBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, light_depth_maps, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error(
                "lightFBO framebuffer "
                "is not complete");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Configure UBO
        glGenBuffers(1, &matrices_UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, matrices_UBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, matrices_UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    ShadowMap::~ShadowMap()
    {
        glDeleteFramebuffers(1, &light_FBO);
        glDeleteBuffers(1, &matrices_UBO);
        glDeleteTextures(1, &light_depth_maps);
    }

    void ShadowMap::bind()
    {
        // UBO setup
        const auto lightMatrices = get_light_space_matrices();

        glBindBuffer(GL_UNIFORM_BUFFER, matrices_UBO);
        for (size_t i = 0; i < lightMatrices.size(); i++)
            glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightMatrices[i]);

        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Render depth of scene to texture (from light's perspective)
        shadow_map_depth->bind();
        glBindFramebuffer(GL_FRAMEBUFFER, light_FBO);
        glViewport(0, 0, depth_map_resolution, depth_map_resolution);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);  // peter panning (dont forget to reset after render)
    }

    void ShadowMap::unbind()
    {
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        shadow_map_depth->unbind();
    }

    void ShadowMap::bind_maps(Shader &shader, u32 slot)
    {
        // Set shadow map uniforms
        shader.set_uniform1("textures.shadowMap", slot);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D_ARRAY, light_depth_maps);

        shader.set_uniform1("cascadeCount", static_cast<u32>(shadow_cascade_levels.size()));

        for (u64 i = 0; i < shadow_cascade_levels.size(); i++)
            shader.set_uniform1("cascadePlaneDistances[" + to_str(i) + "]", shadow_cascade_levels[i]);
    }

    u32 layer = 0;
    bool released = true;
    void ShadowMap::render_debug()
    {
        if (Input::is_key_pressed(KEY_SPACE) && released)
        {
            layer = (layer + 1) % 5;
            released = false;
        }

        if (!Input::is_key_pressed(KEY_SPACE)) released = true;

        // Render Depth map for visual debugging
        debug_depth->bind();
        debug_depth->set_uniform1("layer", layer);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, light_depth_maps);
        debug_quad->render();
    }

    void ShadowMap::set_light_dir(const vec3 &light_dir)
    {
        this->light_dir = light_dir;
    }

    vec3 ShadowMap::get_light_dir()
    {
        return light_dir;
    }

    Shader &ShadowMap::get_shadow_depth_shader() const
    {
        return *shadow_map_depth;
    }

    std::vector<vec4> ShadowMap::get_frustum_corners_world_space(const mat4 &projview)
    {
        const mat4 inv = inverse(projview);

        std::vector<vec4> frustum_corners;
        for (u32 x = 0; x < 2; ++x)
        {
            for (u32 y = 0; y < 2; ++y)
            {
                for (u32 z = 0; z < 2; ++z)
                {
                    const vec4 pt = inv * vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                    frustum_corners.push_back(pt / pt.w);
                }
            }
        }

        return frustum_corners;
    }

    std::vector<vec4> ShadowMap::get_frustum_corners_world_space(const mat4 &proj, const mat4 &view)
    {
        return get_frustum_corners_world_space(proj * view);
    }

    mat4 ShadowMap::get_light_space_matrix(f32 near, f32 far)
    {
        auto &window = Game::get().get_window();
        f32 width = window.get_width();
        f32 height = window.get_height();

        const mat4 proj = perspective(radians(camera.zoom), width / height, near, far);

        const auto corners = get_frustum_corners_world_space(proj, camera.view_matrix);

        vec3 center = vec3(0.0f);
        for (const vec4 &v : corners) center += vec3(v);

        center /= corners.size();

        const mat4 lightView = lookAt(center + light_dir, center, vec3(0.0f, 1.0f, 0.0f));

        f32 minX = std::numeric_limits<f32>::max();
        f32 maxX = std::numeric_limits<f32>::lowest();
        f32 minY = std::numeric_limits<f32>::max();
        f32 maxY = std::numeric_limits<f32>::lowest();
        f32 minZ = std::numeric_limits<f32>::max();
        f32 maxZ = std::numeric_limits<f32>::lowest();

        for (const vec4 &v : corners)
        {
            const vec4 trf = lightView * v;
            minX = min(minX, trf.x);
            maxX = max(maxX, trf.x);
            minY = min(minY, trf.y);
            maxY = max(maxY, trf.y);
            minZ = min(minZ, trf.z);
            maxZ = max(maxZ, trf.z);
        }

        // Tune this parameter according to the scene
        constexpr f32 zMult = 10.0f;
        if (minZ < 0)
            minZ *= zMult;

        else
            minZ /= zMult;

        if (maxZ < 0)
            maxZ /= zMult;

        else
            maxZ *= zMult;

        const mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
        return lightProjection * lightView;
    }

    std::vector<mat4> ShadowMap::get_light_space_matrices()
    {
        std::vector<mat4> ret;
        for (size_t i = 0; i < shadow_cascade_levels.size() + 1; ++i)
        {
            if (i == 0)
                ret.push_back(get_light_space_matrix(camera.near, shadow_cascade_levels[i]));

            else if (i < shadow_cascade_levels.size())
                ret.push_back(get_light_space_matrix(shadow_cascade_levels[i - 1], shadow_cascade_levels[i]));

            else
                ret.push_back(get_light_space_matrix(shadow_cascade_levels[i - 1], camera.far));
        }
        return ret;
    }
};  // namespace bls
