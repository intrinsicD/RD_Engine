#pragma once

#include "assets/AssetHandle.h"
#include "IRenderer.h"

namespace RDE {


    class IAsset {
    public:
        virtual ~IAsset() = default;

        AssetHandle ID{};
    };
}
