//
// Created by alex on 27.08.25.
//

#include "FileDropLayer.h"
#include "core/events/ApplicationEvent.h"
#include "core/Log.h"

#include <imgui.h>

namespace RDE{
    void FileDropLayer::on_attach() {

    }

    void FileDropLayer::on_detach() {

    }

    void FileDropLayer::on_update(float /*delta_time*/) {
        // No update logic needed for file drop layer
    }

    void FileDropLayer::on_render_gui() {
        // if the scene is empty, render a message in the center of the screen saying "Drop files here to load"
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
        ImGui::Begin("FileDropLayer", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                                         | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing
                                         | ImGuiWindowFlags_NoNav);
        ImGui::Text("Drop files here to load");
        ImGui::End();
    }

    void FileDropLayer::on_event(Event &e) {
        // No additional event handling needed
        if(!e.handled){
            EventDispatcher dispatcher(e);
            dispatcher.dispatch<WindowFileDropEvent>([this](WindowFileDropEvent &e) {
                on_file_drop(e.get_files());
                return true; // Mark event as handled
            });
        }
    }

    void FileDropLayer::on_file_drop(const std::vector<std::string> &paths) {
        for (const auto &path : paths) {
            // Handle each dropped file path
            RDE_INFO("File dropped: {}", path);
            // Here you can add logic to process the dropped files, e.g., load assets, open files, etc.
        }
    }
}