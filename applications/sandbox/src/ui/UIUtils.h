#pragma once

#include "Entity.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuiFileDialog.h>
#include <string>

namespace RDE::UI {
    // This is our generic UI drawing helper for a component.
    template<typename T, typename UIFunction>
    static void DrawComponent(const std::string &name, Entity entity, UIFunction ui_function) {
        if (!entity.has_component<T>()) return;

        const ImGuiTreeNodeFlags tree_node_flags =
                ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap |
                ImGuiTreeNodeFlags_SpanAvailWidth;

        auto &component = entity.get_component<T>();
        ImVec2 content_region_available = ImGui::GetContentRegionAvail();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
        float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImGui::Separator();
        bool open = ImGui::TreeNodeEx((void *) typeid(T).hash_code(), tree_node_flags, "%s", name.c_str());
        ImGui::PopStyleVar();

        ImGui::SameLine(content_region_available.x - line_height * 0.5f);
        if (ImGui::Button("+", ImVec2{line_height, line_height})) {
            ImGui::OpenPopup("ComponentSettings");
        }

        bool remove_component = false;
        if (ImGui::BeginPopup("ComponentSettings")) {
            if (ImGui::MenuItem("Remove Component"))
                remove_component = true;
            ImGui::EndPopup();
        }

        if (open) {
            ui_function(component);
            ImGui::TreePop();
        }

        if (remove_component)
            entity.remove_component<T>();
    }

    // Helper to add and remove components in the UI.
    template<typename T>
    static void DrawAddComponentPopupMenuItem(const char *label, Entity entity) {
        if (ImGui::MenuItem(label)) {
            if (!entity.has_component<T>())
                entity.add_component<T>();
            else
                // Optional: Log a warning that component already exists
                // RDE_CORE_WARN("Entity already has a Sprite Renderer Component!");
                ImGui::CloseCurrentPopup();
        }
    }


    // Helper File Dialog for selecting files.
    static bool DrawFileDialog(const char *label, const char *file_extension, std::string &selected_file) {
        if (ImGui::Button(label)) {
            IGFD::FileDialogConfig config;
            config.path = ".";
            config.path = "/home/alex/Dropbox/Work/Datasets";
            ImGuiFileDialog::Instance()->OpenDialog("SelectFile", "Choose File", file_extension, config);
        }
        if (ImGuiFileDialog::Instance()->Display("SelectFile")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                selected_file = ImGuiFileDialog::Instance()->GetFilePathName();
                ImGuiFileDialog::Instance()->Close();
                return true; // File selected
            }
            ImGuiFileDialog::Instance()->Close();
        }
        return false; // No file selected
    }
}
