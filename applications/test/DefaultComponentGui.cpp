#include "scene/DefaultComponentGui.h"
#include "scene/ComponentRegistry.h"

#include "components/TransformInspectorPanelGui.h"
#include "components/CameraInspectorPanelGui.h"

#include "components/TransformComponent.h"
#include "components/CameraComponent.h"
#include "components/BoundingVolumeComponent.h"
#include "components/RenderableComponent.h"
#include "components/MaterialComponent.h"
#include "components/GeometryComponent.h"
#include "components/NameTagComponent.h" // from Core
#include "components/FilepathComponent.h" // from Core

#include <imgui.h>

namespace RDE {
    void RegisterDefaultComponentGui() {
        static bool s_registered = false;
        if (s_registered) return;
        s_registered = true;

        auto &reg = ComponentRegistryGui::instance();

        // NameTagComponent
        reg.register_component<NameTagComponent>(
            "Name",
            [](entt::registry &r, entt::entity e){
                auto &tag = r.get<NameTagComponent>(e);
                char buf[256];
                std::snprintf(buf, sizeof(buf), "%s", tag.name.c_str());
                if (ImGui::InputText("Name", buf, sizeof(buf))) {
                    tag.name = buf;
                }
            },
            [](entt::registry &r, entt::entity e){ r.emplace_or_replace<NameTagComponent>(e, NameTagComponent{"Entity"}); }
        );

        // Transform
        reg.register_component<TransformLocal>(
            "Transform",
            [](entt::registry &r, entt::entity e){
                TransformInspectorPanelGui gui{ r.try_get<TransformLocal>(e), r.try_get<TransformWorld>(e) };
                if (gui) gui.Draw(); else ImGui::TextUnformatted("No Transform present");
            },
            [](entt::registry &r, entt::entity e){ r.emplace_or_replace<TransformLocal>(e); }
        );

        // Camera (grouped via inspector)
        reg.register_component<CameraComponent>(
            "Camera",
            [](entt::registry &r, entt::entity e){
                CameraInspectorPanelGui gui{
                    r.try_get<CameraMatrices>(e),
                    r.try_get<CameraViewParameters>(e),
                    r.try_get<CameraProjectionParameters>(e),
                    r.try_get<CameraFrustumPlanes>(e),
                    r.all_of<CameraDirty>(e),
                    r.all_of<CameraPrimary>(e)
                };
                if (gui) gui.Draw(); else ImGui::TextUnformatted("No Camera data");
            },
            [](entt::registry &r, entt::entity e){ r.emplace_or_replace<CameraComponent>(e); }
        );

        // Renderable
        reg.register_component<RenderableComponent>(
            "Renderable",
            [](entt::registry &r, entt::entity e){
                auto &rc = r.get<RenderableComponent>(e);
                ImGui::Checkbox("Visible", &rc.isVisible);
                ImGui::Text("Geometry Asset: %u", rc.geometry_id ? (unsigned)entt::to_integral(rc.geometry_id->entity_id) : 0u);
            },
            [](entt::registry &r, entt::entity e){ r.emplace_or_replace<RenderableComponent>(e); }
        );

        // Material
        reg.register_component<MaterialComponent>(
            "Material",
            [](entt::registry &r, entt::entity e){
                auto &mc = r.get<MaterialComponent>(e);
                ImGui::Text("Material Asset: %u", mc.material_asset_id ? (unsigned)entt::to_integral(mc.material_asset_id->entity_id) : 0u);
                ImGui::Text("Parameters: %zu", mc.parameters.n_properties());
            },
            [](entt::registry &r, entt::entity e){ r.emplace_or_replace<MaterialComponent>(e); }
        );

        // Bounding Volumes (AABB only for now)
        reg.register_component<BoundingVolumeAABBComponent>(
            "Bounding Volume (AABB)",
            [](entt::registry &r, entt::entity e){
                auto &bv = r.get<BoundingVolumeAABBComponent>(e);
                ImGui::Text("Local min: (%.3f, %.3f, %.3f)", bv.local.min.x, bv.local.min.y, bv.local.min.z);
                ImGui::Text("Local max: (%.3f, %.3f, %.3f)", bv.local.max.x, bv.local.max.y, bv.local.max.z);
                ImGui::Separator();
                ImGui::Text("World min: (%.3f, %.3f, %.3f)", bv.world.min.x, bv.world.min.y, bv.world.min.z);
                ImGui::Text("World max: (%.3f, %.3f, %.3f)", bv.world.max.x, bv.world.max.y, bv.world.max.z);
            },
            [](entt::registry &r, entt::entity e){ r.emplace_or_replace<BoundingVolumeAABBComponent>(e); }
        );

        // Filepath
        reg.register_component<FilepathComponent>(
            "Source File",
            [](entt::registry &r, entt::entity e){
                auto &fp = r.get<FilepathComponent>(e);
                ImGui::Text("Dir: %s", fp.filepath.string().c_str());
                ImGui::Text("Name: %s", fp.filename.c_str());
                ImGui::Text("Ext: %s", fp.extension.c_str());
            },
            [](entt::registry &r, entt::entity e){ (void)r; (void)e; }
        );
    }
}
