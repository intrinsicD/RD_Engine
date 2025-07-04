#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>
#include <typeindex>

namespace RDE {
    using AttributeID = uint32_t;
    constexpr AttributeID INVALID_ATTRIBUTE_ID = static_cast<AttributeID>(-1);

    class AttributeRegistry {
    public:
        AttributeRegistry() = default;

        // The main interface: get an ID for a string. Creates a new one if it doesn't exist.
        AttributeID get_or_create_id(const std::string &name) {
            std::lock_guard<std::mutex> lock(m_mutex);

            // Check if we already have an ID for this name.
            if (auto it = m_name_to_id.find(name); it != m_name_to_id.end()) {
                return it->second;
            }

            // If not, create a new ID.
            AttributeID newId = static_cast<AttributeID>(m_id_to_name.size());
            m_id_to_name.push_back(name);
            m_name_to_id[name] = newId;

            return newId;
        }

        // Optional: for debugging/reflection, get the name back from an ID.
        const std::string &get_name(AttributeID id) const {
            // No lock needed for const access if writes are locked.
            return m_id_to_name.at(id); // .at() for bounds checking
        }

        template<typename T>
        void register_type(const std::string &name) {
            // This is the bridge: connect the C++ type to a string name.
            m_type_to_string[std::type_index(typeid(T))] = name;
        }

        // New type-safe getter
        template<typename T>
        AttributeID get_id() {
            // Find the string for the type, then get the ID for that string.
            auto it = m_type_to_string.find(std::type_index(typeid(T)));
            if (it == m_type_to_string.end()) {
                // Or throw an exception. A type used should have been registered.
                return INVALID_ATTRIBUTE_ID;
            }
            return get_or_create_id(it->second);
        }

    private:
        std::mutex m_mutex;
        std::unordered_map<std::type_index, std::string> m_type_to_string;
        std::unordered_map<std::string, AttributeID> m_name_to_id;
        std::vector<std::string> m_id_to_name;
    };
}