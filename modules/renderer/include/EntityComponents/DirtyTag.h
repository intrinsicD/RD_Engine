#pragma once

namespace RDE{
    template<typename T>
    struct DirtyTag {
        // This tag is used to mark components that need to be updated or processed.
    };
}