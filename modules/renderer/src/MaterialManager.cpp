#include "material/MaterialManager.h"
#include "core/Log.h"

namespace RDE{
    MaterialManager::MaterialManager(MaterialDatabase &database) : m_database(database) {
        // Initialize the material manager with the provided database
    }

    MaterialID MaterialManager::instantiate_material(const MaterialDescription &description){
        auto material_id = m_database.create_material();

        // Emplace the MaterialDescription into the material database
        m_database.get_registry().emplace_or_replace<MaterialDescription>(material_id, description);
        return material_id;
    }

    MaterialID MaterialManager::instantiate_material(const MaterialID &source_material_id){
        auto source_description = get_material_description(source_material_id);
        auto source_gpu_material = get_gpu_material(source_material_id);
        if (!source_description || !source_gpu_material) {
            RDE_CORE_ERROR("Failed to instantiate material: Source material ID is invalid or missing description/GPU material.");
            return MaterialID{}; // Return an invalid MaterialID
        }
        return instantiate_material(*source_description);
    }

    MaterialDescription *MaterialManager::get_material_description(const MaterialID &material_id){
        if( !m_database.get_registry().valid(material_id)) {
            RDE_CORE_ERROR("Attempted to access an invalid material ID: {}", static_cast<uint32_t>(material_id));
            return nullptr; // Return nullptr if the material ID is invalid
        }
        return m_database.get_registry().try_get<MaterialDescription>(material_id);
    }

    const MaterialDescription *MaterialManager::get_material_description(const MaterialID &material_id) const{
        if( !m_database.get_registry().valid(material_id)) {
            RDE_CORE_ERROR("Attempted to access an invalid material ID: {}", static_cast<uint32_t>(material_id));
            return nullptr; // Return nullptr if the material ID is invalid
        }
        return m_database.get_registry().try_get<MaterialDescription>(material_id);
    }

    GpuMaterial *MaterialManager::get_gpu_material(const MaterialID &material_id){
        if( !m_database.get_registry().valid(material_id)) {
            RDE_CORE_ERROR("Attempted to access an invalid material ID: {}", static_cast<uint32_t>(material_id));
            return nullptr; // Return nullptr if the material ID is invalid
        }
        return m_database.get_registry().try_get<GpuMaterial>(material_id);
    }

    const GpuMaterial *MaterialManager::get_gpu_material(const MaterialID &material_id) const{
        if( !m_database.get_registry().valid(material_id)) {
            RDE_CORE_ERROR("Attempted to access an invalid material ID: {}", static_cast<uint32_t>(material_id));
            return nullptr; // Return nullptr if the material ID is invalid
        }
        return m_database.get_registry().try_get<GpuMaterial>(material_id);
    }
}