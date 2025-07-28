#pragma once

#include "ral/Common.h"
#include "core/Properties.h"

namespace RDE{
    struct GpuDataSource{
        RAL::BufferHandle buffer_handle; // Handle to the GPU buffer
        std::string property_name; // Name of the property associated with this data source
    };
    struct CpuDataSource{
        std::string property_name; // Name of the property associated with this data source
    };
    struct PositionDataSource : public CpuDataSource {
        PositionDataSource(const BasePropertyArray *base_property = nullptr) {
            if(base_property) {
                property_name = base_property->name(); // Use the name from the base property
            } else {
                property_name = "v:source_unknown:point"; // Default property name for vertex positions
            }
        }
    };
    struct VertexNormalsDataSource : public CpuDataSource {
        VertexNormalsDataSource(const BasePropertyArray *base_property = nullptr) {
            if(base_property) {
                property_name = base_property->name(); // Use the name from the base property
            } else {
                property_name = "v:source_unknown:normal"; // Default property name for vertex normals
            }
        }
    };
    struct VertexColorsDataSource : public CpuDataSource {
        VertexColorsDataSource(const BasePropertyArray *base_property = nullptr) {
            if(base_property) {
                property_name = base_property->name(); // Use the name from the base property
            } else {
                property_name = "v:source_unknown:color"; // Default property name for vertex colors
            }
        }
    };
    struct TrisDataSource : public CpuDataSource {
        TrisDataSource(const BasePropertyArray *base_property = nullptr) {
            if(base_property) {
                property_name = base_property->name(); // Use the name from the base property
            } else {
                property_name = "f:source_unknown:point"; // Default property name for vertex positions
            }
        }
    };
}