//systems/GpuGeometryUploadSystem.cpp
#include "systems/GpuGeometryUploadSystem.h"
#include "assets/AssetComponentTypes.h"
#include "scene/SystemDependencyBuilder.h"
#include "ral/BufferUploadManager.h"
#include "core/Properties.h"

#include <entt/entity/registry.hpp>

namespace RDE {
    GpuGeometryUploadSystem::GpuGeometryUploadSystem(entt::registry &registry, RAL::Device *device,
                                                     RAL::BufferUploadManager *buffer_upload_manager)
            : m_registry(registry), m_device(device), m_buffer_upload_manager(buffer_upload_manager) {
    }

    void GpuGeometryUploadSystem::init() {
        // Initialization logic for the geometry system
    }

    void GpuGeometryUploadSystem::update(float delta_time) {
        // Create GpuGeometry components from CpuGeometry components
        auto view = m_registry.view<AssetCpuGeometry, Dirty<AssetCpuGeometry>>();
        for (auto entity: view) {
            const auto &dirty_properties = view.get<Dirty<AssetCpuGeometry>>(entity);

            if (dirty_properties.dirty_vertex_properties.empty() &&
                dirty_properties.dirty_halfedge_properties.empty() &&
                dirty_properties.dirty_edge_properties.empty() &&
                dirty_properties.dirty_face_properties.empty() &&
                dirty_properties.dirty_tets_properties.empty()) {
                continue; // No dirty properties, skip processing
            }

            auto &geometry = view.get<AssetCpuGeometry>(entity);
            auto &gpu_geometry = m_registry.get_or_emplace<AssetGpuGeometry>(entity);
            gpu_geometry.subviews = geometry.subviews; // Copy subviews
            // Process geometry data, e.g., convert to GPU format
            // This is where you would typically upload the geometry to the GPU
            // and create an AssetGpuGeometry component if needed.

            // Use the BufferUploadManager to upload the geometry data for all dirty properties
            if (!dirty_properties.dirty_vertex_properties.empty()) {
                for (const auto &property_name: dirty_properties.dirty_vertex_properties) {
                    // Handle each dirty vertex property, e.g., positions, normals, etc.
                    // This is a simplified example; you would need to handle each property type accordingly.
                    auto base_property = geometry.vertices.get(property_name);
                    if (base_property) {
                        // Upload the property data to the GPU
                        auto &buffer_handle = gpu_geometry.buffers[property_name];
                        RAL::BufferUsage usage = RAL::BufferUsage::StorageBuffer;
                        if (property_name == "v:indices") { // Assuming this is your standard name for triangle indices
                            usage |= RAL::BufferUsage::IndexBuffer;
                        } else {
                            usage |= RAL::BufferUsage::VertexBuffer; // For vertex attributes
                        }
                        m_buffer_upload_manager->update_or_create_buffer(buffer_handle,
                                                                         base_property->total_size_bytes(),
                                                                         base_property->data(),
                                                                         usage
                        );
                    }
                }
            }
            if (!dirty_properties.dirty_halfedge_properties.empty()) {
                for (const auto &property_name: dirty_properties.dirty_halfedge_properties) {
                    // Handle each dirty halfedge property
                    auto base_property = geometry.halfedges.get(property_name);
                    if (base_property) {
                        auto &buffer_handle = gpu_geometry.buffers[property_name];
                        RAL::BufferUsage usage = RAL::BufferUsage::StorageBuffer;
                        if (property_name == "h:halfedges" ||
                            property_name == "h:indices") { // Assuming this is your standard name for triangle indices
                            usage |= RAL::BufferUsage::IndexBuffer;
                        }
                        m_buffer_upload_manager->update_or_create_buffer(buffer_handle,
                                                                         base_property->total_size_bytes(),
                                                                         base_property->data(),
                                                                         usage
                        );
                    }
                }
            }
            if (!dirty_properties.dirty_edge_properties.empty()) {
                for (const auto &property_name: dirty_properties.dirty_edge_properties) {
                    // Handle each dirty edge property
                    auto base_property = geometry.edges.get(property_name);
                    if (base_property) {
                        auto &buffer_handle = gpu_geometry.buffers[property_name];
                        RAL::BufferUsage usage = RAL::BufferUsage::StorageBuffer;
                        if (property_name == "e:edges" ||
                            property_name == "e:indices") { // Assuming this is your standard name for triangle indices
                            usage |= RAL::BufferUsage::IndexBuffer;
                        }
                        m_buffer_upload_manager->update_or_create_buffer(buffer_handle,
                                                                         base_property->total_size_bytes(),
                                                                         base_property->data(),
                                                                         usage
                        );
                    }
                }
            }
            if (!dirty_properties.dirty_face_properties.empty()) {
                for (const auto &property_name: dirty_properties.dirty_face_properties) {
                    // Handle each dirty face property
                    auto base_property = geometry.faces.get(property_name);
                    if (base_property) {
                        auto &buffer_handle = gpu_geometry.buffers[property_name];
                        RAL::BufferUsage usage = RAL::BufferUsage::StorageBuffer;
                        if (property_name == "f:tris" ||
                            property_name == "f:indices") { // Assuming this is your standard name for triangle indices
                            usage |= RAL::BufferUsage::IndexBuffer;
                        }
                        m_buffer_upload_manager->update_or_create_buffer(buffer_handle,
                                                                         base_property->total_size_bytes(),
                                                                         base_property->data(),
                                                                         usage
                        );
                    }
                }
            }
            if (!dirty_properties.dirty_tets_properties.empty()) {
                for (const auto &property_name: dirty_properties.dirty_tets_properties) {
                    // Handle each dirty tets property
                    auto base_property = geometry.tets.get(property_name);
                    if (base_property) {
                        auto &buffer_handle = gpu_geometry.buffers[base_property->name()];
                        RAL::BufferUsage usage = RAL::BufferUsage::StorageBuffer;
                        if (property_name == "t:tets" ||
                            property_name == "t:indices") { // Assuming this is your standard name for triangle indices
                            usage |= RAL::BufferUsage::IndexBuffer;
                        }
                        m_buffer_upload_manager->update_or_create_buffer(buffer_handle,
                                                                         base_property->total_size_bytes(),
                                                                         base_property->data(),
                                                                         usage
                        );
                    }
                }
            }
            m_buffer_upload_manager->flush();
            // After processing, clear the dirty properties
            m_registry.remove<Dirty<AssetCpuGeometry>>(entity); // Clear dirty flags
        }
    }

    void GpuGeometryUploadSystem::shutdown() {
        // Cleanup logic for the geometry system
    }

    void GpuGeometryUploadSystem::declare_dependencies(SystemDependencyBuilder &builder) {
        // Declare dependencies for this system
        //builder.reads<>();
    }
}