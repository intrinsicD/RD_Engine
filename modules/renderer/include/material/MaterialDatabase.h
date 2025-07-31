//renderer/materials/MaterialDatabase.h
#pragma once

#include "material/MaterialID.h"
#include <entt/entity/registry.hpp>

namespace RDE {
    class MaterialDatabase {
    public:
        MaterialDatabase() = default;

        ~MaterialDatabase() = default;

        template<typename MaterialComponentType>
        MaterialComponentType &get(const MaterialID& material_id) {
            if (!m_registry.valid(material_id)) {
                throw std::runtime_error("Attempted to access an invalid entity in the material database.");
            }
            return m_registry.get<MaterialComponentType>(material_id);
        }


        template<typename MaterialComponentType>
        MaterialComponentType *try_get(const MaterialID& material_id) {
            if (!m_registry.valid(material_id)) {
                return nullptr;
            }
            const entt::entity entity = material_id;
            if (!m_registry.valid(entity)) {
                return nullptr;
            }
            return m_registry.try_get<MaterialComponentType>(entity);
        }

        void destroy_material(const MaterialID& material_id) {
            m_registry.destroy(material_id);
        }

        entt::registry &get_registry() {
            return m_registry;
        }

        const entt::registry &get_registry() const {
            return m_registry;
        }

    private:
        friend class MaterialManager;

        entt::entity create_material() {
            // Create a new entity in the registry for the material
            return m_registry.create();
        }

        template<typename... Args>
        auto emplace(entt::entity entity, Args &&... args) {
            return m_registry.emplace<Args...>(entity, std::forward<Args>(args)...);
        }

        template<typename... Args>
        auto emplace_or_replace(entt::entity entity, Args &&... args) {
            return m_registry.emplace_or_replace<Args...>(entity, std::forward<Args>(args)...);
        }

        entt::registry m_registry;
    };
}
