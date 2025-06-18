// RDE_Project/modules/core/src/ImGuiLayer.cpp

#include "ImGuiLayer.h"
#include "Application.h"
#include <imgui.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

namespace RDE {
    ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {
    }

    ImGuiLayer::~ImGuiLayer() {
    }

    void ImGuiLayer::on_attach() {
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

        Application &app = Application::get();
        Window &window = app.get_window();

        ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow *>(window.get_native_window()), true);
        ImGui_ImplOpenGL3_Init("#version 410");
    }

    void ImGuiLayer::on_detach() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::on_event(Event &e) {
        ImGuiIO &io = ImGui::GetIO();
        // Block events from layers below if ImGui is using the mouse/keyboard
        e.handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
        e.handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
    }


    void ImGuiLayer::begin() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::end() {
        ImGuiIO &io = ImGui::GetIO();
        // TEMPORARY: Should get window size from our own Window class.
        Application &app = Application::get();
        Window &window = app.get_window();

        int width, height;
        glfwGetWindowSize(static_cast<GLFWwindow *>(window.get_native_window()), &width, &height);
        io.DisplaySize = ImVec2((float) width, (float) height);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }
}