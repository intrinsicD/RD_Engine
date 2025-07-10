//vulkan/VulkanDeletionQueue.h

#pragma once

#include <deque>
#include <functional>
#include <vector>

namespace RDE {
    class DeletionQueue {
    public:
        void push(std::function<void()> &&function);

        void flush();

    private:
        std::deque<std::function<void()>> m_deletors;
    };
}