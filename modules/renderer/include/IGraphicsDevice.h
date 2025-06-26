#pragma once

#include "RendererTypes.h"

#include <cstdint>

namespace RDE {


    class IGraphicsDevice {
    public:
        virtual ~IGraphicsDevice() = default;

        virtual GpuGeometryHandle create_geometry(const GeometryDesc &geometry_data) = 0;

        virtual GpuTextureHandle create_texture(const TextureDesc &texture_data) = 0;

        virtual GpuMaterialHandle create_material(const MaterialDesc &material_data) = 0;

        virtual GpuProgramHandle create_program(const ProgramDesc &desc) = 0;

        virtual GpuShaderHandle create_shader(const ShaderModuleDesc &desc) = 0;

        virtual GpuBufferHandle create_buffer(const BufferDesc &buffer_data) = 0;

        virtual void destroy_geometry(GpuGeometryHandle handle) = 0;

        virtual void destroy_texture(GpuTextureHandle handle) = 0;

        virtual void destroy_material(GpuMaterialHandle handle) = 0;

        virtual void destroy_program(GpuProgramHandle handle) = 0;

        virtual void destroy_buffer(GpuBufferHandle handle) = 0;
    };
}

namespace std {
    template<RDE::GpuHandleType T>
    struct hash<RDE::GpuHandle<T> > {
        size_t operator()(const RDE::GpuHandle<T> &handle) const {
            return hash<uint64_t>()(handle.id);
        }
    };
} // namespace std