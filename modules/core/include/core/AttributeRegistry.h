#pragma once

    #include <unordered_map>
    #include <vector>
    #include <string>
    #include <mutex>
    #include <typeindex>

    namespace RDE {
        // AttributeID is a unique identifier for an attribute name.
        using AttributeID = uint32_t;
        // Special value indicating an invalid or unregistered attribute.
        constexpr AttributeID INVALID_ATTRIBUTE_ID = static_cast<AttributeID>(-1);

        /**
         * @brief AttributeRegistry manages the mapping between string attribute names,
         *        unique integer IDs, and C++ types.
         *
         * This class allows you to:
         *  - Register a string name for an attribute and get a unique ID for it.
         *  - Retrieve the name from an ID (useful for debugging or reflection).
         *  - Associate C++ types with attribute names for type-safe lookups.
         *
         * Thread safety: All write operations are protected by a mutex.
         */
        class AttributeRegistry {
        public:
            /**
             * @brief Constructs an empty AttributeRegistry.
             */
            AttributeRegistry() = default;

            /**
             * @brief Get the unique ID for a given attribute name.
             *        If the name is not registered, a new ID is created and returned.
             *
             * @param name The string name of the attribute.
             * @return AttributeID The unique ID associated with the name.
             *
             * Usage:
             *   AttributeID id = registry.get_or_create_id("position");
             */
            AttributeID get_or_create_id(const std::string &name) {
                std::lock_guard<std::mutex> lock(m_mutex);

                // Check if the name is already registered.
                if (auto it = m_name_to_id.find(name); it != m_name_to_id.end()) {
                    return it->second;
                }

                // Register a new ID for this name.
                AttributeID newId = static_cast<AttributeID>(m_id_to_name.size());
                m_id_to_name.push_back(name);
                m_name_to_id[name] = newId;

                return newId;
            }

            AttributeID get(const std::string &name) {
                std::lock_guard<std::mutex> lock(m_mutex);

                // Check if the name is already registered.
                if (auto it = m_name_to_id.find(name); it != m_name_to_id.end()) {
                    return it->second;
                }

                return INVALID_ATTRIBUTE_ID;
            }

            /**
             * @brief Retrieve the attribute name associated with a given ID.
             *
             * @param id The attribute ID.
             * @return const std::string& The name corresponding to the ID.
             * @throws std::out_of_range if the ID is invalid.
             *
             * Usage:
             *   std::string name = registry.get_name(id);
             */
            const std::string &get_name(AttributeID id) const {
                // No lock needed for const access if writes are locked.
                return m_id_to_name.at(id); // .at() throws if id is out of range
            }

            /**
             * @brief Register a C++ type with a string attribute name.
             *
             * This allows you to later retrieve the attribute ID using the type.
             *
             * @tparam T The C++ type to register.
             * @param name The string name to associate with the type.
             *
             * Usage:
             *   registry.register_type<MyComponent>("my_component");
             */
            template<typename T>
            void register_type(const std::string &name) {
                // Map the C++ type to the attribute name.
                m_type_to_string[std::type_index(typeid(T))] = name;
            }

            /**
             * @brief Get the attribute ID associated with a registered C++ type.
             *
             * @tparam T The C++ type.
             * @return AttributeID The ID for the type's attribute name, or INVALID_ATTRIBUTE_ID if not registered.
             *
             * Usage:
             *   AttributeID id = registry.get_id<MyComponent>();
             */
            template<typename T>
            AttributeID get_id() {
                // Look up the string name for the type, then get its ID.
                auto it = m_type_to_string.find(std::type_index(typeid(T)));
                if (it == m_type_to_string.end()) {
                    // Type not registered.
                    return INVALID_ATTRIBUTE_ID;
                }
                return get_or_create_id(it->second);
            }

        private:
            // Mutex to ensure thread-safe access to the registry.
            std::mutex m_mutex;
            // Maps C++ type_index to attribute name.
            std::unordered_map<std::type_index, std::string> m_type_to_string;
            // Maps attribute name to unique ID.
            std::unordered_map<std::string, AttributeID> m_name_to_id;
            // Maps ID to attribute name (for reverse lookup).
            std::vector<std::string> m_id_to_name;
        };
    }