#include "VulkanDeletionQueue.h"

namespace RDE {
    void DeletionQueue::push(std::function<void()> &&function) {
        m_deletors.push_back(std::move(function));
    }

    void DeletionQueue::flush() {
        // Reverse iterate and call the functions to delete resources in reverse order of creation.
        for (auto it = m_deletors.rbegin(); it != m_deletors.rend(); ++it) {
            (*it)(); // Call the lambda
        }
        m_deletors.clear();
    }
}