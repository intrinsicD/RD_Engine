#pragma once

#include "core/ILayer.h"
#include "assets/AssetDatabase.h"

namespace RDE {
    class AssetViewerLayer : public ILayer {
    public:
        explicit AssetViewerLayer(AssetDatabase *asset_database) : m_asset_database(asset_database) {}

        void on_attach() override {};

        void on_detach() override {};

        void on_update(float delta_time) override {};

        void on_render(RAL::CommandBuffer *cmd) override {};

        void on_render_gui() override;

        void on_event(Event &e) override {};

        const char *get_name() const override {
            return m_name.c_str();
        }

    private:
        std::string m_name = "AssetViewerLayer"; // Name of the layer
        AssetDatabase *m_asset_database = nullptr; // Pointer to the asset database
    };
}