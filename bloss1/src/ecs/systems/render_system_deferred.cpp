#include "config.hpp"
#include "core/game.hpp"
#include "ecs/ecs.hpp"
#include "ecs/systems/render_system.hpp"
#include "renderer/height_map.hpp"
#include "renderer/model.hpp"
#include "renderer/post/post_processing.hpp"
#include "renderer/shadow_map.hpp"
#include "renderer/skybox.hpp"
#include "tools/profiler.hpp"

namespace bls
{
    void render_system_deferred(ECS &ecs, f32 dt)
    {
        BLS_PROFILE_SCOPE("render_system_deferred");

        auto &window = Game::get().get_window();
        auto &renderer = Game::get().get_renderer();

        auto width = window.get_width();
        auto height = window.get_height();

        auto camera = ecs.cameras.begin()->second.get();
        auto position = camera->position;
        auto projection = camera->projection_matrix;
        auto view = camera->view_matrix;
        auto near = camera->near;
        auto far = camera->far;

        auto &shaders = renderer.get_shaders();
        auto &textures = renderer.get_textures();
        auto &g_buffer = renderer.get_gbuffer();
        auto &skybox = renderer.get_skybox();
        auto &quad = renderer.get_rendering_quad();
        auto &shadow_map = renderer.get_shadow_map();
        auto &height_map = renderer.get_height_map();
        auto &post_processing = renderer.get_post_processing();

        // Shaders - by now they should have been initialized
        auto g_buffer_shader = shaders["g_buffer"].get();
        auto pbr_shader = shaders["pbr"].get();

        // Reset the viewport
        renderer.clear_color({0.0f, 0.0f, 0.0f, 1.0f});
        renderer.clear();
        renderer.set_viewport(0, 0, width, height);

        // Render shadow map
        shadow_map->bind();
        render_scene(ecs, shadow_map->get_shadow_depth_shader(), renderer);
        shadow_map->unbind();

        // Reset the viewport
        renderer.clear_color({0.0f, 0.0f, 0.0f, 1.0f});
        renderer.clear();
        renderer.set_viewport(0, 0, width, height);

        // Geometry pass: render scene data into gbuffer
        // -------------------------------------------------------------------------------------------------------------
        g_buffer->bind();
        renderer.clear();

        g_buffer_shader->bind();
        g_buffer_shader->set_uniform4("projection", projection);
        g_buffer_shader->set_uniform4("view", view);

        // Render the scene
        render_scene(ecs, *g_buffer_shader, renderer);

        // Render height map
        if (AppConfig::tess_wireframe)
        {
            renderer.set_debug_mode(AppConfig::tess_wireframe);
            height_map->render(view, projection, dt);
            renderer.set_debug_mode(!AppConfig::tess_wireframe);
        }

        else
            height_map->render(view, projection, dt);

        // Render particles
        particle_system(ecs, dt);

        g_buffer->unbind();
        g_buffer_shader->unbind();

        // Lighting pass: calculate lighting using the gbuffer content
        // -------------------------------------------------------------------------------------------------------------
        renderer.clear();

        pbr_shader->bind();

        // Set camera position
        pbr_shader->set_uniform3("viewPos", position);

        // Set light uniforms
        pbr_shader->set_uniform3("lightDir", shadow_map->get_light_dir());
        pbr_shader->set_uniform1("near", near);
        pbr_shader->set_uniform1("far", far);

        // Set lights uniforms
        u32 light_counter = 0;

        // Point lights
        auto &point_lights = ecs.point_lights;
        auto &transforms = ecs.transforms;
        for (auto &[id, light] : point_lights)
        {
            auto transform = transforms[id].get();

            pbr_shader->set_uniform3("lights.pointLightPositions[" + to_str(light_counter) + "]", transform->position);
            pbr_shader->set_uniform3("lights.pointLightColors[" + to_str(light_counter) + "]", light->diffuse);

            light_counter++;
        }

        // Directional lights
        light_counter = 0;
        auto &dir_lights = ecs.dir_lights;
        for (auto &[id, light] : dir_lights)
        {
            auto transform = transforms[id].get();

            pbr_shader->set_uniform3("lights.dirLightDirections[" + to_str(light_counter) + "]", transform->rotation);
            pbr_shader->set_uniform3("lights.dirLightColors[" + to_str(light_counter) + "]", light->diffuse);

            light_counter++;
        }

        // Set texture attachments ---
        u32 tex_position = 0;
        for (const auto &[name, texture] : textures)
        {
            if (name == "ui") continue;

            pbr_shader->set_uniform1("textures." + name, tex_position);
            texture->bind(tex_position);
            tex_position++;
        }

        // Bind maps
        skybox->bind(*pbr_shader, 12);           // IBL maps
        shadow_map->bind_maps(*pbr_shader, 15);  // Shadow map

        // Begin post processing process
        post_processing->begin();
        quad->render();  // Render light quad
        post_processing->end();

        // Copy content of geometry's depth buffer to default framebuffer's depth buffer
        // -------------------------------------------------------------------------------------------------------------
        g_buffer->bind_and_blit(width, height);
        g_buffer->unbind();

        // Draw the skybox last
        skybox->draw(view, projection);

        // Draw UI
        render_ui();

// Render debug lines
#if !defined(_RELEASE)
        if (AppConfig::render_colliders) render_colliders(ecs, projection, view);
#endif

        // Render texts
        render_texts(ecs);
    }
};  // namespace bls
