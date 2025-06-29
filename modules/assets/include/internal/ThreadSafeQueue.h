#pragma once
#include <queue>
#include <mutex>
#include <optional>

namespace RDE {
    template<typename T>
    class ThreadSafeQueue {
    public:
        void push(T value) {
            std::lock_guard lock(m_mutex);
            m_queue.push(std::move(value));
        }

        std::optional<T> try_pop() {
            std::lock_guard lock(m_mutex);
            if (m_queue.empty()) {
                return std::nullopt;
            }
            T value = std::move(m_queue.front());
            m_queue.pop();
            return value;
        }

    private:
        std::queue<T> m_queue;
        std::mutex m_mutex;
    };
}
