//ral/ResourceDatabase.h
#pragma once

#include "ral/RenderHandle.h"
#include <entt/entity/registry.hpp>

namespace RAL {
    class ResourcesDatabase {
        struct Generation {
            uint32_t generation = 0; // Incremented on creation
        };
    public:
        ResourcesDatabase() = default;

        ~ResourcesDatabase() = default;

        constexpr bool is_valid(const RenderHandle &handle) const {
            return handle.is_valid() && m_registry.valid(handle.index) &&
                   m_registry.get<Generation>(handle.index).generation == entt::to_version(handle.index);
        }

        RenderHandle create() {
            auto entity = m_registry.create();
            m_registry.emplace<Generation>(entity, entt::to_version(entity));
            return RenderHandle{entity};
        }

        void destroy(const RenderHandle &handle) {
            m_registry.destroy(handle.index);
        }

        template<typename... Type>
        [[nodiscard]] decltype(auto) get([[maybe_unused]] const RenderHandle &handle) {
            return m_registry.get<Type...>(handle.index);
        }

        template<typename... Type>
        [[nodiscard]] auto try_get([[maybe_unused]] const RenderHandle &handle) const {
            return m_registry.try_get<Type...>(handle.index);
        }

        template<typename Type, typename... Args>
        [[nodiscard]] decltype(auto) get_or_emplace(const RenderHandle &handle, Args &&...args) {
            return m_registry.get_or_emplace<Type>(handle.index, std::forward<Args>(args)...);
        }

        template<typename Type, typename... Args>
        decltype(auto) emplace(const RenderHandle &handle, Args &&...args) {
            return m_registry.emplace<Type>(handle.index, std::forward<Args>(args)...);
        }

        template<typename Type, typename... Args>
        decltype(auto) emplace_or_replace(const RenderHandle &handle, Args &&...args) {
            return m_registry.emplace_or_replace<Type>(handle.index, std::forward<Args>(args)...);
        }

        entt::registry &get_registry() {
            return m_registry;
        }

        const entt::registry &get_registry() const {
            return m_registry;
        }

    private:
        entt::registry m_registry;
    };
}
