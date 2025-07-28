//systems/GpuGeometryUploadSystem.h
#pragma once

#include "core/ISystem.h"
#include "ral/Device.h"
#include "ral/BufferUploadManager.h"

#include <entt/fwd.hpp>

namespace RDE {
    class PropertyContainer;

    class GpuGeometryUploadSystem : public ISystem {
    public:
        GpuGeometryUploadSystem(entt::registry &registry, RAL::Device *device,
                                RAL::BufferUploadManager *buffer_upload_manager);


        ~GpuGeometryUploadSystem() override = default;

        void init() override;

        void update(float delta_time) override;

        void shutdown() override;

        void declare_dependencies(SystemDependencyBuilder &builder) override;

    private:
        entt::registry &m_registry;
        RAL::Device *m_device; // Reference to the GPU device for uploading geometry
        RAL::BufferUploadManager *m_buffer_upload_manager; // Reference to the GPU device for uploading geometry
    };
}