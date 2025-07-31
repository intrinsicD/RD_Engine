#pragma once

#include "MaterialID.h"
#include "MaterialDatabase.h"
#include "MaterialDescription.h"

#include <optional>

namespace RDE{
    class MaterialManager {
    public:
        explicit MaterialManager(MaterialDatabase &database);

        MaterialID instantiate_material(const MaterialDescription &description);

        MaterialID instantiate_material(const MaterialID &source_material_id);

        MaterialDescription *get_material_description(const MaterialID &material_id);

        const MaterialDescription *get_material_description(const MaterialID &material_id) const;

        GpuMaterial *get_gpu_material(const MaterialID &material_id);

        const GpuMaterial *get_gpu_material(const MaterialID &material_id) const;

    private:
        MaterialDatabase &m_database;
    };
}