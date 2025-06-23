#pragma once
#include <vector>

namespace RDE::Components {
    struct BoneInfo {
        //TODO
    };
    struct SkeletonComponent {
        std::vector<BoneInfo> bones;
    };
}
