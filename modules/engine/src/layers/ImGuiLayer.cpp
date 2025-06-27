// RDE_Project/modules/core/src/ImGuiLayer.cpp

#include "ImGuiLayer.h"
#include "IWindow.h"
#include "ApplicationContext.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

namespace RDE {
    ImGuiLayer::ImGuiLayer() : ILayer("ImGuiLayer") {
    }

    void ImGuiLayer::on_attach(const ApplicationContext &app_context, const FrameContext &frame_context) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        auto &window = *app_context.window;
        ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow *>(window.get_native_window()), true);
        ImGui_ImplOpenGL3_Init("#version 410");
    }

    void ImGuiLayer::on_detach(const ApplicationContext &, const FrameContext &) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::on_event(Event &e, const ApplicationContext &, const FrameContext &) {
        ImGuiIO &io = ImGui::GetIO();
        // Block events from layers below if ImGui is using the mouse/keyboard
        e.handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
        e.handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
    }


    void ImGuiLayer::begin(const ApplicationContext &, const FrameContext &) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::BeginMainMenuBar();
    }

    void ImGuiLayer::end(const ApplicationContext &app_context, const FrameContext &) {
        ImGui::EndMainMenuBar();
        ImGuiIO &io = ImGui::GetIO();

        auto &window = *app_context.window;
        unsigned int width = window.get_width();
        unsigned int height = window.get_height();
        io.DisplaySize = ImVec2((float) width, (float) height);

        ImGui::Render();
        //this is now handled in the imgui pass

/*        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }*/
    }
}