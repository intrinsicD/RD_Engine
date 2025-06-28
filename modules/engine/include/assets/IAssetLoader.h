#pragma once

#include "IAsset.h"

#include <memory>
#include <entt/fwd.hpp>

namespace RDE{
    class IAssetLoader {
    public:
        virtual ~IAssetLoader() = default;

        virtual void load(const std::string &uri, entt::registry &asset_registry, entt::entity asset_id) = 0;

        const std::vector<std::string> &get_supported_extension() const { return m_supported_extension; }

    private:
        std::vector<std::string> m_supported_extension; // The file extension this loader handles, e.g., ".png", ".obj", etc.
    };
}