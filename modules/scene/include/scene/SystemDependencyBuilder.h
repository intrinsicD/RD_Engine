#pragma once

#include <typeindex>
#include <vector>

namespace RDE {
    class SystemDependencyBuilder {
    public:
        SystemDependencyBuilder() = default;

        template<typename T>
        void reads() {
            m_reads.emplace_back(typeid(T));
        }

        template<typename T>
        void writes() {
            m_writes.emplace_back(typeid(T));
        }

        const std::vector <std::type_index> &get_reads() {
            return m_reads;
        }

        const std::vector <std::type_index> &get_writes() {
            return m_writes;
        }

    private:
        std::vector <std::type_index> m_reads;
        std::vector <std::type_index> m_writes;
    };
}