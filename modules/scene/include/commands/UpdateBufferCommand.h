#pragma once

#include "ral/Common.h"
#include <vector>

namespace RDE::Commands {
    struct UpdateBufferCommand {
        RAL::BufferHandle target_buffer  = RAL::BufferHandle::INVALID();

        std::vector<uint8_t> data;
        uint64_t destinationOffset = 0;
    };
}