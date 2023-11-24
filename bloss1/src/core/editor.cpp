#include "core/editor.hpp"

#include "core/game.hpp"
#include "core/logger.hpp"
#include "ecs/scene_parser.hpp"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "platform/glfw/window.hpp"
#include "renderer/height_map.hpp"
#include "renderer/model.hpp"
#include "renderer/post/post_processing.hpp"

namespace bls
{
    Editor::Editor(Window &window)
        : window(window),
          save_file("bloss1/assets/scenes/test_stage2.bloss"),
          config_file("bloss1/assets/scenes/bloss_config2.bcfg")
    {
        // Context creation
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto &io = ImGui::GetIO();
        (void)io;

        // Nice font :)
        io.Fonts->AddFontFromFileTTF("bloss1/assets/fonts/JetBrainsMono-Regular.ttf", 15);

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        // Style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsClassic();

        ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow *>(window.get_native_window()), true);
        ImGui_ImplOpenGL3_Init("#version 460");
    }

    Editor::~Editor()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void Editor::update(ECS &ecs, f32)
    {
        // Create ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        auto &io = ImGui::GetIO();
        (void)io;

        // Styles
        push_style_vars();

        // Dockspace
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(),
                                     ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoWindowMenuButton |
                                         ImGuiDockNodeFlags_NoCloseButton);

        // Options
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Options"))
            {
                ImGui::InputTextWithHint("##", "file", save_file, 64);
                ImGui::SameLine();
                if (ImGui::SmallButton("Save Scene")) SceneParser::save_scene(ecs, save_file);

                ImGui::InputTextWithHint("##", "file", config_file, 64);
                ImGui::SameLine();
                if (ImGui::SmallButton("Save Config")) SceneParser::save_config(config_file);

                ImGui::Separator();
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        // Application status
        AppStats::framerate = io.Framerate;
        AppStats::ms_per_frame = 1000.0f / io.Framerate;
        render_status();

        // Configuration parameters
        render_config();

        // Entities
        render_entities(ecs);

        pop_style_vars();

        // Render
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    void Editor::render_status()
    {
        ImGui::Begin("Status");

        // Show performance plots
        ImGui::Text("Frame time: %.3f ms/frame (%.0f FPS)", AppStats::ms_per_frame, AppStats::framerate);
        {
            static std::vector<f32> frame_values(100, 0.0f);
            if (frame_values.size() == 100) frame_values.erase(frame_values.begin());
            frame_values.push_back(AppStats::ms_per_frame);

            ImGui::PlotHistogram("##",
                                 frame_values.data(),
                                 static_cast<i32>(frame_values.size()),
                                 0,
                                 NULL,
                                 0.0f,
                                 33.3f,
                                 ImVec2(0, 60.0f));

            // ImGui::PlotLines(
            //     "##", frame_values.data(), static_cast<i32>(frame_values.size()), 0, NULL, 0.0f, 33.3f,
            //     ImVec2(0, 60.0f));
        }

        ImGui::Text("Vertices: %u", AppStats::vertices);
        {
            static std::vector<f32> vert_values(100, 0.0f);
            if (vert_values.size() == 100) vert_values.erase(vert_values.begin());
            vert_values.push_back(AppStats::vertices);

            ImGui::PlotHistogram("##",
                                 vert_values.data(),
                                 static_cast<i32>(vert_values.size()),
                                 0,
                                 NULL,
                                 0.0f,
                                 5'000'000.0f,
                                 ImVec2(0, 60.0f));
        }

        ImGui::End();

        // Reset stats
        AppStats::framerate = {};
        AppStats::ms_per_frame = {};
        AppStats::vertices = {};
    }

    void Editor::render_config()
    {
        auto &renderer = Game::get().get_renderer();

        ImGui::Begin("Configuration");

        if (ImGui::CollapsingHeader("Debug"))
        {
            ImGui::Dummy(ImVec2(10.0f, 10.0f));
            ImGui::Text("Debug Options");
            ImGui::Separator();
            ImGui::Dummy(ImVec2(10.0f, 10.0f));
            ImGui::Checkbox("Colliders", &AppConfig::render_colliders);
            ImGui::Checkbox("Tesselation Wireframe", &AppConfig::tess_wireframe);
            ImGui::Dummy(ImVec2(10.0f, 10.0f));
        }

        if (ImGui::CollapsingHeader("Terrain"))
        {
            auto &height_map = renderer.get_height_map();

            ImGui::Dummy(ImVec2(10.0f, 10.0f));
            ImGui::Text("Terrain Options");
            ImGui::Separator();
            ImGui::Dummy(ImVec2(10.0f, 10.0f));
            ImGui::InputInt("Min tesselation level", reinterpret_cast<i32 *>(&height_map->min_tess_level));
            ImGui::InputInt("Max tesselation level", reinterpret_cast<i32 *>(&height_map->max_tess_level));
            ImGui::InputFloat("Min distance", &height_map->min_distance);
            ImGui::InputFloat("Max distance", &height_map->max_distance);
            ImGui::InputFloat2("Displacement Multiplier", value_ptr(height_map->displacement_multiplier));
            ImGui::InputInt("Noise Algorithm", reinterpret_cast<i32 *>(&height_map->noise_algorithm));
            ImGui::Dummy(ImVec2(10.0f, 10.0f));

            ImGui::Text("Layers");
            ImGui::Separator();
            ImGui::Dummy(ImVec2(10.0f, 10.0f));

            for (size_t i = 0; i < height_map->texture_heights.size(); i++)
                ImGui::InputFloat(("Layer " + to_str(i)).c_str(), &height_map->texture_heights[i]);

            ImGui::Checkbox("Toggle Gradient", &height_map->toggle_gradient);
            ImGui::Dummy(ImVec2(10.0f, 10.0f));

            ImGui::Text("FBM");
            ImGui::Separator();
            ImGui::Dummy(ImVec2(10.0f, 10.0f));
            ImGui::InputInt("Octaves", &height_map->fbm_octaves);
            ImGui::InputFloat("FBM Scale", &height_map->fbm_scale);
            ImGui::InputFloat("FBM Height", &height_map->fbm_height);
            ImGui::Dummy(ImVec2(10.0f, 10.0f));

            ImGui::Text("Perlin");
            ImGui::Separator();
            ImGui::Dummy(ImVec2(10.0f, 10.0f));
            ImGui::InputFloat("Perlin Scale", &height_map->perlin_scale);
            ImGui::InputFloat("Perlin Height", &height_map->perlin_height);
            ImGui::Dummy(ImVec2(10.0f, 10.0f));
        }

        if (ImGui::CollapsingHeader("Post Processing"))
        {
            ImGui::Dummy(ImVec2(10.0f, 10.0f));
            ImGui::Text("Passes");
            ImGui::Separator();
            ImGui::Dummy(ImVec2(10.0f, 10.0f));

            ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersH |
                                    ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_BordersInnerH |
                                    ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterV |
                                    ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuter |
                                    ImGuiTableFlags_BordersInner | ImGuiTableFlags_Resizable;
            //    ImGuiTableFlags_NoBordersInBody;

            ImGui::BeginTable("render_passes_table", 5, flags);

            ImGui::TableSetupColumn("ID");
            ImGui::TableSetupColumn("Position");
            ImGui::TableSetupColumn("Pass");
            ImGui::TableSetupColumn("Parameters");
            ImGui::TableSetupColumn("Active");

            ImGui::TableHeadersRow();
            auto &post_processing = renderer.get_post_processing();
            for (auto &pass : AppConfig::render_passes)
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text(to_str(pass.id).c_str());

                ImGui::TableSetColumnIndex(1);
                if (pass.id > 0)
                {
                    if (!ImGui::InputInt(("position_" + to_str(pass.id)).c_str(),
                                         reinterpret_cast<i32 *>(&pass.position)))
                        pass.position = clamp(pass.position, 1U, static_cast<u32>(AppConfig::render_passes.size()));
                }

                else
                    ImGui::Text(("position_" + to_str(pass.id)).c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::Text(pass.name.c_str());

                ImGui::TableSetColumnIndex(3);
                display_editable_params(pass);

                ImGui::TableSetColumnIndex(4);
                if (pass.id > 0)
                    ImGui::Checkbox(("enabled_" + to_str(pass.id)).c_str(), &pass.enabled);

                else
                    ImGui::Text("BasePass is always enabled");

                if (pass.id > 0) post_processing->set_pass(pass.id, pass.enabled, pass.position);
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }

    void Editor::display_editable_params(PassConfig &pass)
    {
        if (typeid(*pass.pass) == typeid(FogPass))
        {
            auto *fog_pass = static_cast<FogPass *>(pass.pass);
            ImGui::InputFloat3("Fog Color", value_ptr(fog_pass->fog_color));
            ImGui::InputFloat3("Fog Min/Max Range", value_ptr(fog_pass->min_max));
        }

        else if (typeid(*pass.pass) == typeid(BloomPass))
        {
            auto *bloom_pass = static_cast<BloomPass *>(pass.pass);
            ImGui::InputInt("Bloom Samples", reinterpret_cast<i32 *>(&bloom_pass->samples));
            ImGui::InputFloat("Bloom Spread", &bloom_pass->spread);
            ImGui::InputFloat("Bloom Threshold", &bloom_pass->threshold);
            ImGui::InputFloat("Bloom Amount", &bloom_pass->amount);
        }

        else if (typeid(*pass.pass) == typeid(SharpenPass))
        {
            auto *sharpen_pass = static_cast<SharpenPass *>(pass.pass);
            ImGui::InputFloat("Sharpen Amount", &sharpen_pass->amount);
        }

        else if (typeid(*pass.pass) == typeid(PosterizationPass))
        {
            auto *poster_pass = static_cast<PosterizationPass *>(pass.pass);
            ImGui::InputFloat("Poster Levels", &poster_pass->levels);
        }

        else if (typeid(*pass.pass) == typeid(PixelizationPass))
        {
            auto *pixel_pass = static_cast<PixelizationPass *>(pass.pass);
            ImGui::InputInt("Pixel Size", reinterpret_cast<i32 *>(&pixel_pass->pixel_size));
        }

        else if (typeid(*pass.pass) == typeid(OutlinePass))
        {
            auto *outline_pass = static_cast<OutlinePass *>(pass.pass);
            ImGui::InputFloat("Threshold", &outline_pass->threshold);
            ImGui::InputFloat3("Edge Color", value_ptr(outline_pass->color));
        }

        else if (typeid(*pass.pass) == typeid(VignettePass))
        {
            auto *vignette_pass = static_cast<VignettePass *>(pass.pass);
            ImGui::InputFloat("Lens Radius", &vignette_pass->lens_radius);
            ImGui::InputFloat("Lens Feathering", &vignette_pass->lens_feathering);
        }

        else if (typeid(*pass.pass) == typeid(KuwaharaPass))
        {
            auto *kuwahara_pass = static_cast<KuwaharaPass *>(pass.pass);
            ImGui::InputInt("Radius", reinterpret_cast<i32 *>(&kuwahara_pass->radius));
        }

        else
            ImGui::Text("-");
    }

    void Editor::render_entities(ECS &ecs)
    {
        // Scene entities
        ImGui::Begin("Entities");

        for (const auto &[id, name] : ecs.names)
        {
            if (!ImGui::CollapsingHeader((name + "_" + to_str(id)).c_str())) continue;

            ImGui::Text("id: %d", id);
            ImGui::Separator();
            ImGui::Dummy(ImVec2(10.0f, 10.0f));

            if (ecs.transforms.count(id))
            {
                ImGui::Text("transform");
                ImGui::Separator();
                ImGui::InputFloat3("position", value_ptr(ecs.transforms[id]->position));
                ImGui::InputFloat3("rotation", value_ptr(ecs.transforms[id]->rotation));
                ImGui::InputFloat3("scale", value_ptr(ecs.transforms[id]->scale));
                ImGui::Dummy(ImVec2(10.0f, 10.0f));
            }

            if (ecs.models.count(id))
            {
                ImGui::Text("model");
                ImGui::Separator();
                ImGui::Text("path: %s", ecs.models[id]->model->path.c_str());

                for (const auto &[animation_name, animation] : ecs.models[id]->model->animations)
                    ImGui::Text("animation: %s", animation_name.c_str());
                ImGui::Dummy(ImVec2(10.0f, 10.0f));
            }

            if (ecs.physics_objects.count(id))
            {
                ImGui::Text("physics object");
                ImGui::Separator();
                ImGui::InputFloat3("force", value_ptr(ecs.physics_objects[id]->force));
                ImGui::InputFloat3("velocity", value_ptr(ecs.physics_objects[id]->velocity));
                ImGui::InputFloat3("terminal velocity", value_ptr(ecs.physics_objects[id]->terminal_velocity));
                ImGui::InputFloat("mass", &ecs.physics_objects[id]->mass);
                ImGui::Dummy(ImVec2(10.0f, 10.0f));
            }

            if (ecs.colliders.count(id))
            {
                ImGui::Text("collider");
                ImGui::Separator();
                ImGui::Text("type: %s", Collider::get_collider_str(ecs.colliders[id]->type).c_str());

                if (ecs.colliders[id]->type == Collider::ColliderType::Sphere)
                {
                    auto *radius = &static_cast<SphereCollider *>(ecs.colliders[id].get())->radius;
                    ImGui::InputFloat("radius", radius);
                }

                else if (ecs.colliders[id]->type == Collider::ColliderType::Box)
                {
                    auto &dimensions = static_cast<BoxCollider *>(ecs.colliders[id].get())->dimensions;
                    ImGui::InputFloat3("dimensions", value_ptr(dimensions));
                }

                ImGui::InputFloat3("offset", value_ptr(ecs.colliders[id]->offset));
                ImGui::Checkbox("immovable", &ecs.colliders[id]->immovable);
                ImGui::Dummy(ImVec2(10.0f, 10.0f));
            }

            if (ecs.dir_lights.count(id))
            {
                ImGui::Text("dir light");
                ImGui::Separator();
                ImGui::InputFloat3("ambient", value_ptr(ecs.dir_lights[id]->ambient));
                ImGui::InputFloat3("diffuse", value_ptr(ecs.dir_lights[id]->diffuse));
                ImGui::InputFloat3("specular", value_ptr(ecs.dir_lights[id]->specular));
                ImGui::Dummy(ImVec2(10.0f, 10.0f));
            }

            if (ecs.point_lights.count(id))
            {
                ImGui::Text("point light");
                ImGui::Separator();
                ImGui::InputFloat3("ambient", value_ptr(ecs.point_lights[id]->ambient));
                ImGui::InputFloat3("diffuse", value_ptr(ecs.point_lights[id]->diffuse));
                ImGui::InputFloat3("specular", value_ptr(ecs.point_lights[id]->specular));

                ImGui::InputFloat("constant", &ecs.point_lights[id]->constant);
                ImGui::InputFloat("linear", &ecs.point_lights[id]->linear);
                ImGui::InputFloat("quadratic", &ecs.point_lights[id]->quadratic);

                ImGui::Dummy(ImVec2(10.0f, 10.0f));
            }

            if (ecs.cameras.count(id))
            {
                ImGui::Text("camera");
                ImGui::Separator();
                ImGui::InputFloat3("target offset", value_ptr(ecs.cameras[id]->target_offset));
                ImGui::InputFloat3("world up", value_ptr(ecs.cameras[id]->world_up));

                ImGui::InputFloat("zoom", &ecs.cameras[id]->target_zoom);
                ImGui::InputFloat("near", &ecs.cameras[id]->near);
                ImGui::InputFloat("far", &ecs.cameras[id]->far);
                ImGui::InputFloat("lerp factor", &ecs.cameras[id]->lerp_factor);

                ImGui::Dummy(ImVec2(10.0f, 10.0f));
            }

            if (ecs.camera_controllers.count(id))
            {
                ImGui::Text("camera controller");
                ImGui::Separator();
                ImGui::InputFloat("sensitivity", &ecs.camera_controllers[id]->sensitivity);
                ImGui::InputFloat3("speed", value_ptr(ecs.camera_controllers[id]->speed));

                ImGui::Dummy(ImVec2(10.0f, 10.0f));
            }

            if (ecs.texts.count(id))
            {
                auto font_file = ecs.texts[id]->font_file;
                auto &text = ecs.texts[id]->text;

                std::vector<char> text_c(text.c_str(), text.c_str() + text.size() + 1);
                char buffer[512];
                strcpy(buffer, text_c.data());

                ImGui::Text("text");
                ImGui::Separator();
                ImGui::Text(("font: " + font_file).c_str());
                ImGui::InputText("text", buffer, 512);
                ImGui::InputFloat3("color", value_ptr(ecs.texts[id]->color));

                text = buffer;

                ImGui::Dummy(ImVec2(10.0f, 10.0f));
            }

            if (ecs.sounds.count(id))
            {
                for (auto &[sound_name, sound] : ecs.sounds[id])
                {
                    ImGui::Text("sound");
                    ImGui::Separator();
                    ImGui::Text(("name: " + sound_name).c_str());
                    ImGui::Text(("file: " + sound->file).c_str());
                    ImGui::Text(("looping: " + to_str(sound->looping)).c_str());
                    ImGui::InputFloat("volume ", &sound->volume);
                    ImGui::Checkbox("play now ", &sound->play_now);

                    ImGui::Dummy(ImVec2(10.0f, 10.0f));
                }
            }

            if (ecs.timers.count(id))
            {
                auto &time = ecs.timers[id]->time;

                ImGui::Text("timer");
                ImGui::Separator();
                ImGui::InputFloat("time", &time);

                ImGui::Dummy(ImVec2(10.0f, 10.0f));
            }

            if (ecs.transform_animations.count(id))
            {
                auto &key_frames = ecs.transform_animations[id]->key_frames;

                ImGui::Text("transform_animation");
                ImGui::Separator();
                for (u32 i = 0; i < key_frames.size(); i++)
                {
                    auto &key_frame = key_frames[i];

                    ImGui::Text(("frame: " + to_str(i)).c_str());
                    ImGui::InputFloat3(("position_" + to_str(i)).c_str(), value_ptr(key_frame.transform.position));
                    ImGui::InputFloat3(("rotation_" + to_str(i)).c_str(), value_ptr(key_frame.transform.rotation));
                    ImGui::InputFloat3(("scale_" + to_str(i)).c_str(), value_ptr(key_frame.transform.scale));
                    ImGui::InputFloat(("duration_" + to_str(i)).c_str(), &key_frame.duration);
                    ImGui::Dummy(ImVec2(5.0f, 5.0f));
                }

                ImGui::Dummy(ImVec2(10.0f, 10.0f));
            }

            if (ecs.hitpoints.count(id))
            {
                auto &hitpoints = ecs.hitpoints[id];

                ImGui::Text("hitpoints");
                ImGui::Separator();
                ImGui::InputFloat(("hitpoints_" + to_str(id)).c_str(), &hitpoints);
                ImGui::Dummy(ImVec2(10.0f, 10.0f));
            }

            if (ecs.particle_systems.count(id))
            {
                ImGui::Text("particle_system");
                ImGui::Separator();

                const auto &particle_sys = ecs.particle_systems[id];
                if (particle_sys->emitter->type == Emitter::EmitterType::Box)
                {
                    const auto &emitter = particle_sys->emitter;

                    ImGui::Text("type: %s", "box");
                    auto &dimensions = static_cast<BoxEmitter *>(emitter.get())->dimensions;
                    ImGui::InputFloat3("dimensions", value_ptr(dimensions));
                }

                else if (ecs.colliders[id]->type == Collider::ColliderType::Sphere)
                {
                    const auto &emitter = particle_sys->emitter;

                    ImGui::Text("type: %s", "sphere");
                    auto *radius = &static_cast<SphereEmitter *>(emitter.get())->radius;
                    ImGui::InputFloat("radius", radius);
                }

                ImGui::Text("emitter");
                ImGui::Checkbox("particle_2D", &particle_sys->emitter->particle_2D);
                ImGui::InputFloat3("center", value_ptr(particle_sys->emitter->center));
                ImGui::InputInt("particles to be emitted",
                                reinterpret_cast<i32 *>(&particle_sys->particles_to_be_emitted));
                ImGui::InputFloat("time to emit", &particle_sys->time_to_emit);
                ImGui::Dummy(ImVec2(10.0f, 10.0f));

                ImGui::Text("particle");
                auto particle = particle_sys->emitter->get_particle();
                ImGui::InputFloat("lifetime", &particle.life_time);
                ImGui::InputFloat4("color begin", value_ptr(particle.color_begin));
                ImGui::InputFloat4("color end", value_ptr(particle.color_end));
                ImGui::InputFloat3("scale begin", value_ptr(particle.scale_begin));
                ImGui::InputFloat3("scale end", value_ptr(particle.scale_end));
                ImGui::InputFloat3("velocity", value_ptr(particle.velocity));
                ImGui::InputFloat3("velocity variation", value_ptr(particle.velocity_variation));
                particle_sys->emitter->set_particle(particle);

                ImGui::Dummy(ImVec2(10.0f, 10.0f));
            }
        }

        ImGui::End();
    }

    void Editor::push_style_vars()
    {
        // Styling
        const ImVec4 fgColor = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
        const ImVec4 bgColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        const ImVec4 bgSecondaryColor = ImVec4(0.2f, 0.4f, 0.4f, 1.0f);
        const ImVec4 bgTertiaryColor = ImVec4(0.2f, 0.7f, 0.7f, 1.0f);

        // Colors
        ImGui::PushStyleColor(ImGuiCol_WindowBg, bgColor);
        ImGui::PushStyleColor(ImGuiCol_TitleBg, bgSecondaryColor);
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, bgSecondaryColor);

        ImGui::PushStyleColor(ImGuiCol_Header, bgSecondaryColor);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, bgSecondaryColor);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, bgSecondaryColor);

        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, bgSecondaryColor);
        ImGui::PushStyleColor(ImGuiWindowDockStyleCol_Tab, bgColor);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bgColor);

        ImGui::PushStyleColor(ImGuiCol_Tab, bgSecondaryColor);
        ImGui::PushStyleColor(ImGuiCol_TabHovered, bgSecondaryColor);
        ImGui::PushStyleColor(ImGuiCol_TabActive, bgSecondaryColor);
        ImGui::PushStyleColor(ImGuiCol_TabUnfocused, bgSecondaryColor);
        ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, bgSecondaryColor);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, bgSecondaryColor);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, bgSecondaryColor);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, bgSecondaryColor);

        ImGui::PushStyleColor(ImGuiCol_SliderGrab, bgTertiaryColor);
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, bgTertiaryColor);

        ImGui::PushStyleColor(ImGuiCol_PopupBg, bgColor);

        ImGui::PushStyleColor(ImGuiCol_Separator, fgColor);
        ImGui::PushStyleColor(ImGuiCol_SeparatorActive, fgColor);
        ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, fgColor);

        ImGui::PushStyleColor(ImGuiCol_CheckMark, bgTertiaryColor);
        ImGui::PushStyleColor(ImGuiCol_Button, bgSecondaryColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgSecondaryColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgTertiaryColor);

        // Vars
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 2.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 2.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 3.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(6.0f, 4.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 20.0f);
    }

    void Editor::pop_style_vars()
    {
        const u32 colorCounter = 27;
        const u32 varCounter = 11;

        ImGui::PopStyleColor(colorCounter);
        ImGui::PopStyleVar(varCounter);
    }
};  // namespace bls
