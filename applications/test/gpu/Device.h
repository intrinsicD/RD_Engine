#pragma once

#include <daxa>

namespace RDE {
#define GPU_HANDLE(Name) struct Name { uint64_t id = 0; bool is_valid() const { return id != 0; } };

    GPU_HANDLE(PipelineHandle);

    GPU_HANDLE(BufferHandle);

    GPU_HANDLE(TextureHandle);

    GPU_HANDLE(ProgramHandle);

    GPU_HANDLE(DescriptorSetHandle);

    GPU_HANDLE(SemaphoreHandle);

    struct BufferDesc{
        uint64_t size = 0; // Size in bytes
        uint32_t usage = 0; // Usage flags (e.g., GPU_READ, GPU_WRITE)
        const char *name = nullptr; // Optional name for debugging
    };

    struct TextureDesc{
        uint32_t width = 0; // Width in pixels
        uint32_t height = 0; // Height in pixels
        uint32_t depth = 1; // Depth for 3D textures
        uint32_t format = 0; // Format (e.g., R8G8B8A8_UNORM)
        uint32_t usage = 0; // Usage flags (e.g., GPU_READ, GPU_WRITE)
        const char *name = nullptr; // Optional name for debugging
    };

    struct ProgramDesc{
        const char *vertex_shader_source = nullptr; // Vertex shader source code
        const char *fragment_shader_source = nullptr; // Fragment shader source code
        const char *geometry_shader_source = nullptr; // Optional geometry shader source code
        const char *tessellation_control_shader_source = nullptr; // Optional tessellation control shader source code
        const char *tessellation_evaluation_shader_source = nullptr; // Optional tessellation evaluation shader source code
        const char *compute_shader_source = nullptr; // Optional compute shader source code
        const char *name = nullptr; // Optional name for debugging
    };

    class Device {
    public:
        virtual ~Device() = default;

        // Resource management
        virtual BufferHandle create_buffer(const BufferDesc &) = 0;

        virtual void destroy_buffer(BufferHandle handle) = 0;

        virtual TextureHandle create_texture(const TextureDesc &) = 0;

        virtual void destroy_texture(TextureHandle handle) = 0;

        virtual ProgramHandle create_program(const ProgramDesc &) = 0;

        virtual void destroy_program(ProgramHandle handle) = 0;

        // TODO later: Command submission
        virtual void submit_commands() = 0;

        // TODO later:  Synchronization
        virtual void wait_for_idle() = 0;

    private:
    };
}