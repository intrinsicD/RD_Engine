#pragma once

#include "ral/Common.h"

namespace RDE::Commands{
    struct CopyBufferCommand {
        RAL::BufferHandle sourceBuffer = RAL::BufferHandle::INVALID();
        RAL::BufferHandle targetBuffer = RAL::BufferHandle::INVALID();
        uint64_t sourceOffset = 0;
        uint64_t targetOffset = 0;
        uint64_t size = -1; // -1 indicates copy the entire buffer
    };
}