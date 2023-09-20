#include "editor.hpp"
#include "platform/glfw/window.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "renderer/model.hpp"

namespace bls
{
    Editor::Editor(Window& window) : window(window)
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

        std::cout << "editor destroyed successfully\n";
    }

    void Editor::update(ECS& ecs, f32 dt)
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
                if (ImGui::MenuItem("This is another menu item", "Hint", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) { }

                ImGui::Separator();
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        // Application status
        ImGui::Begin("Status");
        ImGui::Text("Frame time: %.3f ms/frame (%.0f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();

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

    void Editor::render_entities(ECS& ecs)
    {
        // Scene entities
        ImGui::Begin("Entities");

        // @TODO: oooofff
        for (const auto& [id, name] : ecs.names)
        {
            if (!ImGui::CollapsingHeader(name.c_str()))
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

                // @TODO: convert Box dimensions to vec3
                else if (ecs.colliders[id]->type == Collider::ColliderType::Box)
                {
                    vec3 dimensions =
                    {
                        static_cast<BoxCollider*>(ecs.colliders[id].get())->width,
                        static_cast<BoxCollider*>(ecs.colliders[id].get())->height,
                        static_cast<BoxCollider*>(ecs.colliders[id].get())->depth
                    };
                    ImGui::InputFloat3("dimensions", value_ptr(dimensions));
                }

                ImGui::Checkbox("immovable", &ecs.colliders[id]->immovable);
                ImGui::InputFloat3("offset", value_ptr(ecs.colliders[id]->offset));
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
