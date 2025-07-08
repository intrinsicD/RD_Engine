//vulkan/VulkanResourceManager.h
#pragma once

#include "ral/Common.h" // For ResourceSlot

#include <vector>
#include <cstdint>
#include <stdexcept>

namespace RDE {
    // A generic, generation-based resource manager.
    template<typename ResourceType, typename HandleType>
    class ResourceManager {
    public:
        // Creates a new resource and returns a handle to it.
        HandleType create(ResourceType &&resource) {
            uint32_t index;

            if (m_FreeIndices.empty()) {
                index = static_cast<uint32_t>(m_Slots.size());
                m_Slots.emplace_back();
            } else {
                index = m_FreeIndices.back();
                m_FreeIndices.pop_back();
            }

            m_Slots[index].resource = std::move(resource);
            m_Slots[index].generation++; // Increment generation on new creation

            return {index, m_Slots[index].generation};
        }

        // Retrieves a resource by its handle.
        // Throws if the handle is invalid or stale.
        ResourceType &get(HandleType handle) {
            if (!handle.is_valid()) {
                throw std::runtime_error("Invalid resource handle provided.");
            }
            return m_Slots[handle.index].resource;
        }

        // Destroys a resource, freeing its slot for reuse.
        void destroy(HandleType handle) {
            if (!is_valid(handle)) {
                // Attempting to destroy an invalid handle is a no-op.
                return;
            }

            // Note: The actual resource destruction (e.g., vkDestroyBuffer)
            // must happen outside this class. This class only manages the slot.
            m_FreeIndices.push_back(handle.index);
        }

        // Checks if a handle is currently valid.
        bool is_valid(HandleType handle) const {
            return handle.is_valid() &&
                   handle.index < m_Slots.size() &&
                   m_Slots[handle.index].generation == handle.generation;
        }

    private:
        // Use the ResourceSlot we defined in RAL/Common.h
        std::vector<RAL::ResourceSlot<ResourceType>> m_Slots;
        std::vector<uint32_t> m_FreeIndices;
    };
}