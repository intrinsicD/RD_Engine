#pragma once

#include "ral/Device.h"
#include <unordered_map>
#include <vector>

namespace RDE {

    // Internal struct to hold OpenGL specific data for a resource
    struct OpenGL_Texture {
        unsigned int id = 0;
        TextureDesc desc;
    };

    struct OpenGL_Buffer {
        unsigned int id = 0;
        BufferDesc desc;
    };

    struct OpenGL_Program {
        unsigned int id = 0;
        ProgramDesc desc;
    };

    struct OpenGL_Pipeline {
        // In OpenGL, a pipeline is not a single object. We store the description
        // and apply the state when it's bound.
        GraphicsPipelineDesc desc;
    };


class OpenGLDevice : public RAL::Device {
    public:
        OpenGLDevice() = default;

        ~OpenGLDevice() override;

        // --- Resource Creation ---
        GpuGeometryHandle create_geometry(const GeometryDesc &desc) override;

        GpuTextureHandle create_texture(const TextureDesc &desc) override;

        GpuProgramHandle create_program(const ProgramDesc &desc) override;

        GpuBufferHandle create_buffer(const BufferDesc &desc) override;

        GpuPipelineHandle create_graphics_pipeline(const GraphicsPipelineDesc &desc) override;
        // NOTE: We've omitted create_material for now, as it's a higher-level concept
        // that just groups other handles, which the renderer can manage directly.

        // --- Resource Destruction ---
        void destroy_geometry(GpuGeometryHandle handle) override;

        void destroy_texture(GpuTextureHandle handle) override;

        void destroy_program(GpuProgramHandle handle) override;

        void destroy_buffer(GpuBufferHandle handle) override;

        void destroy_graphics_pipeline(GpuPipelineHandle handle) override;

        // --- OpenGL Specific Lookups (used by OpenGLCommandBuffer) ---
        // These allow the command buffer to get the real GLuint from a handle.
        const OpenGL_Texture &get_texture(GpuTextureHandle handle) const;

        const OpenGL_Buffer &get_buffer(GpuBufferHandle handle) const;

        const OpenGL_Program &get_program(GpuProgramHandle handle) const;

        const OpenGL_Pipeline &get_pipeline(GpuPipelineHandle handle) const;

    private:
        // Helper to generate a new handle
        template<GpuHandleType T>
        GpuHandle<T> next_handle();

        uint64_t m_next_handle_id = 1;

        // Resource storage: map from our engine's handle to the backend object
        std::unordered_map<uint64_t, OpenGL_Texture> m_textures;
        std::unordered_map<uint64_t, OpenGL_Buffer> m_buffers;
        std::unordered_map<uint64_t, unsigned int> m_geometries; // Geometry is just a VAO
        std::unordered_map<uint64_t, OpenGL_Program> m_programs;
        std::unordered_map<uint64_t, OpenGL_Pipeline> m_pipelines;
    };

}