#pragma once

#include "core/ILayer.h"
#include "assets/AssetManager.h"
#include "ral/Device.h"
#include "ral/Resources.h"

namespace RDE {
    class TestSceneLayer : public ILayer {
    public:
        TestSceneLayer(AssetManager *asset_manager, entt::registry &registry, RAL::Device *device);

        ~TestSceneLayer() override;

        void on_attach() override;

        void on_detach() override;

        void on_update(float delta_time) override;

        void on_event(Event &e) override;

        void on_render(RAL::CommandBuffer *cmd) override;

        void on_render_gui() override;

        const char *get_name() const override {
            return "TestSceneLayer";
        }

    private:
        void create_test_scene();

        void create_triangle_resources();

        void destroy_triangle_resources();

        AssetManager *m_asset_manager = nullptr; // Pointer to the AssetManager for loading assets
        entt::registry &m_registry; // Reference to the registry for entity management
        RAL::Device *m_device = nullptr; // NEW

        // Triangle resources
        RAL::BufferHandle m_triangleVertexBuffer; // NEW
        RAL::PipelineHandle m_trianglePipeline; // NEW
        RAL::ShaderHandle m_triangleVS; // NEW
        RAL::ShaderHandle m_triangleFS; // NEW
    };
}