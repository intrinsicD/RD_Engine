//renderer/components/GpuBuffers.h
#pragma once

#include "ral/Common.h"
#include <string>
#include <unordered_map>

namespace RDE {
    struct GpuBufferView {
        RAL::BufferHandle buffer_handle; // Handle to the GPU buffer
        size_t size; // Size of the buffer in bytes
        size_t offset;
        std::string name;
    };

    struct GpuBufferComponent {
        std::unordered_map<std::string, GpuBufferView> buffers;
    };
}
