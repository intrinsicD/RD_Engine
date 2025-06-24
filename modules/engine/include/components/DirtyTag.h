#pragma once

namespace RDE::Components {
    template<typename T>
    struct DirtyTag {
        DirtyTag() = default;
        char _ = 0;
    };
}
