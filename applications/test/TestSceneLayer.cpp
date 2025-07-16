#include "TestSceneLayer.h"
#include "core/Paths.h"
#include "components/TransformComponent.h"
#include "components/MaterialComponent.h"
#include "components/HierarchyComponent.h"
#include "components/RenderableComponent.h"
#include "assets/AssetComponentTypes.h"

namespace RDE {
    TestSceneLayer::TestSceneLayer(AssetManager *asset_manager, entt::registry &registry) : m_asset_manager(
        asset_manager), m_registry(registry) {
        // Constructor logic if needed
    }

    TestSceneLayer::~TestSceneLayer() {
        // Destructor logic if needed
    }

    void TestSceneLayer::on_attach() {
        create_test_scene();
    }

    void TestSceneLayer::on_detach() {
        // Cleanup logic if needed
    }

    void TestSceneLayer::on_update(float delta_time) {
        // Update logic for the test scene
    }

    void TestSceneLayer::on_event(Event &e) {
        // Handle events for the test scene
    }

    void TestSceneLayer::on_render(RAL::CommandBuffer *cmd) {
        // Render logic for the test scene
        // This would typically involve binding the pipeline, setting up descriptor sets, etc.
    }

    void TestSceneLayer::on_render_gui() {
        // GUI rendering logic for the test scene
        // This could include ImGui calls to display scene information, controls, etc.
    }

    void TestSceneLayer::create_test_scene() {
        // 1. Load/Create Assets (this is pseudocode, adapt to your AssetManager)
        // You need a simple material (shader, pipeline) and a mesh (cube, etc.)
        auto path = get_asset_path();
        if (!path.has_value()) {
            RDE_ERROR("Failed to get asset path");
            return;
        }

        AssetID cube_mesh_id = m_asset_manager->load(path.value() / "meshes" / "venus.obj");
        AssetID basic_material_id = m_asset_manager->load(path.value() / "materials" / "basic.mat");

        // 2. Create an entity
        auto entity = m_registry.create();

        // 3. Add components
        auto &local_transform = m_registry.emplace<TransformLocal>(entity);
        m_registry.emplace<RenderableComponent>(entity, cube_mesh_id); // Assuming get_id() returns AssetID
        m_registry.emplace<MaterialComponent>(entity, basic_material_id);
        m_registry.emplace<Hierarchy>(entity); // If needed
    }
}
