#include "core/editor.hpp"
#include "core/logger.hpp"
#include "platform/glfw/window.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "renderer/model.hpp"
#include "ecs/scene_parser.hpp"

// bool ImGui::BeginTable(const char* str_id, int columns_count, ImGuiTableFlags flags, const ImVec2& outer_size, float inner_width);
// void ImGui::EndTable();

namespace bls
{
    Editor::Editor(Window& window) : window(window), save_file("bloss1/assets/scenes/test_stage2.bloss")
    {
        // Context creation
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto& io = ImGui::GetIO(); (void) io;

        // Nice font :)
        io.Fonts->AddFontFromFileTTF("bloss1/assets/fonts/JetBrainsMono-Regular.ttf", 15);

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        // Style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsClassic();

        ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(window.get_native_window()), true);
        ImGui_ImplOpenGL3_Init("#version 460");
    }

    Editor::~Editor()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void Editor::update(ECS& ecs, f32)
    {
        // Create ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        auto& io = ImGui::GetIO(); (void) io;

        // Styles
        push_style_vars();

        // Dockspace
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton);

        // Options
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Options"))
            {
                ImGui::InputTextWithHint("##", "file", save_file, 64); ImGui::SameLine();
                if (ImGui::SmallButton("save"))
                    SceneParser::save_scene(ecs, save_file);

                // if (ImGui::MenuItem("This is another menu item", "Hint", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0))
                // {

                // }

                ImGui::Separator();
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        // Application status
        app_stats.framerate = io.Framerate;
        app_stats.ms_per_frame = 1000.0f / io.Framerate;
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
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    void Editor::update_configs(Configs configs)
    {
        this->app_configs = configs;
    }

    void Editor::render_status()
    {
        ImGui::Begin("Status");
        ImGui::Text("Frame time: %.3f ms/frame (%.0f FPS)", app_stats.ms_per_frame, app_stats.framerate);
        ImGui::Text("Vertices: %u", app_stats.vertices);
        ImGui::End();

        // Reset stats
        app_stats = { };
    }

    void Editor::render_config()
    {
        ImGui::Begin("Configuration");
        ImGui::Text("Post processing passes");
        
        bool show = true;
        ImGuiTableFlags flags = ImGuiTableFlags_Borders |
                                ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_Borders |
                                ImGuiTableFlags_BordersH |
                                ImGuiTableFlags_BordersOuterH |
                                ImGuiTableFlags_BordersInnerH |
                                ImGuiTableFlags_BordersV |
                                ImGuiTableFlags_BordersOuterV |
                                ImGuiTableFlags_BordersInnerV |
                                ImGuiTableFlags_BordersOuter |
                                ImGuiTableFlags_BordersInner;
                            //    ImGuiTableFlags_NoBordersInBody;

        ImGui::BeginTable("render_passes_table", 3, flags);
        
        ImGui::TableSetupColumn("Active");
        ImGui::TableSetupColumn("Position");
        ImGui::TableSetupColumn("Pass");

        ImGui::TableHeadersRow();
        for (const auto& [position, name] : app_configs.render_passes)
        {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Checkbox("##", &show);

            ImGui::TableSetColumnIndex(1);
            ImGui::Text(std::to_string(position).c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text(name.c_str());
        }
        ImGui::EndTable();
        ImGui::End();
    }

    void Editor::render_entities(ECS& ecs)
    {
        // Scene entities
        ImGui::Begin("Entities");

        for (const auto& [id, name] : ecs.names)
        {
            if (!ImGui::CollapsingHeader((name + "_" + to_str(id)).c_str()))
                continue;

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

                for (const auto& [animation_name, animation] : ecs.models[id]->model->animations)
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
                    auto* radius = &static_cast<SphereCollider*>(ecs.colliders[id].get())->radius;
                    ImGui::InputFloat("radius", radius);
                }

                else if (ecs.colliders[id]->type == Collider::ColliderType::Box)
                {
                    auto& dimensions = static_cast<BoxCollider*>(ecs.colliders[id].get())->dimensions;
                    ImGui::InputFloat3("dimensions", value_ptr(dimensions));
                }

                ImGui::Checkbox("immovable", &ecs.colliders[id]->immovable);
                ImGui::InputFloat3("offset", value_ptr(ecs.colliders[id]->offset));
                ImGui::Dummy(ImVec2(10.0f, 10.0f));
            }

            if (ecs.dir_lights.count(id))
            {
                ImGui::Text("dir light");
                ImGui::Separator();
                ImGui::InputFloat3("ambient",  value_ptr(ecs.dir_lights[id]->ambient));
                ImGui::InputFloat3("diffuse",  value_ptr(ecs.dir_lights[id]->diffuse));
                ImGui::InputFloat3("specular", value_ptr(ecs.dir_lights[id]->specular));
                ImGui::Dummy(ImVec2(10.0f, 10.0f));
            }

            if (ecs.point_lights.count(id))
            {
                ImGui::Text("point light");
                ImGui::Separator();
                ImGui::InputFloat3("ambient",  value_ptr(ecs.point_lights[id]->ambient));
                ImGui::InputFloat3("diffuse",  value_ptr(ecs.point_lights[id]->diffuse));
                ImGui::InputFloat3("specular", value_ptr(ecs.point_lights[id]->specular));

                ImGui::InputFloat("constant",  &ecs.point_lights[id]->constant);
                ImGui::InputFloat("linear",    &ecs.point_lights[id]->linear);
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
                ImGui::InputFloat("far",  &ecs.cameras[id]->far);
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
                auto& text = ecs.texts[id]->text;

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
                for (auto& [sound_name, sound] : ecs.sounds[id])
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
                auto& time = ecs.timers[id]->time;

                ImGui::Text("timer");
                ImGui::Separator();
                ImGui::InputFloat("time", &time);

                ImGui::Dummy(ImVec2(10.0f, 10.0f));
            }

            if (ecs.transform_animations.count(id))
            {
                auto& key_frames = ecs.transform_animations[id]->key_frames;

                ImGui::Text("transform_animation");
                ImGui::Separator();
                for (u32 i = 0; i < key_frames.size(); i++)
                {
                    auto& key_frame = key_frames[i];

                    ImGui::Text(("frame: " + to_str(i)).c_str());
                    ImGui::InputFloat3(("position_" + to_str(i)).c_str(), value_ptr(key_frame.transform.position));
                    ImGui::InputFloat3(("rotation_" + to_str(i)).c_str(), value_ptr(key_frame.transform.rotation));
                    ImGui::InputFloat3(("scale_" + to_str(i)).c_str(), value_ptr(key_frame.transform.scale));
                    ImGui::InputFloat(("duration_" + to_str(i)).c_str(), &key_frame.duration);
                    ImGui::Dummy(ImVec2(5.0f, 5.0f));
                }

                ImGui::Dummy(ImVec2(10.0f, 10.0f));
            }
        }

        ImGui::End();
    }

    void Editor::push_style_vars()
    {
        // Styling
        const ImVec4 fgColor          = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
        const ImVec4 bgColor          = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        const ImVec4 bgSecondaryColor = ImVec4(0.2f, 0.4f, 0.4f, 1.0f);
        const ImVec4 bgTertiaryColor  = ImVec4(0.2f, 0.7f, 0.7f, 1.0f);

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
};
