#pragma once

#include <sstream>
#include <algorithm>
#include <string>
#include <vector>
#include <cassert>
#include <memory>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace RDE {
    class BasePropertyArray {
    public:
        virtual ~BasePropertyArray() = default;

        virtual void reserve(size_t n) = 0;

        virtual void resize(size_t n) = 0;

        virtual void free_memory() = 0;

        virtual void push_back() = 0;

        virtual void swap(size_t i0, size_t i1) = 0;

        [[nodiscard]] virtual std::unique_ptr<BasePropertyArray> clone() const = 0;

        [[nodiscard]] virtual const std::string &name() const = 0;

        [[nodiscard]] virtual std::string to_string() const = 0;

        [[nodiscard]] virtual std::string to_string(size_t i) const = 0;

        [[nodiscard]] virtual size_t size() const = 0;

        [[nodiscard]] virtual size_t dims() const = 0;

        [[nodiscard]] virtual const void *data() const = 0;

        [[nodiscard]] virtual size_t total_size_bytes() = 0;
    };

    struct StringTraits {
        template<typename T>
        static std::string ToString(const T &t) {
            std::stringstream ss;
            ss << t;
            return ss.str();
        }

        // Overload for bool
        static std::string ToString(bool t) {
            return t ? "true" : "false";
        }

        // Overload for glm::vec
        template<glm::length_t L, typename T, glm::qualifier Q>
        static std::string ToString(const glm::vec<L, T, Q>& value) {
            return glm::to_string(value);
        }

        // Overload for glm::mat
        template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
        static std::string ToString(const glm::mat<C, R, T, Q>& value) {
            return glm::to_string(value);
        }

        // Overload for std::vector. This now calls our other to_string overloads.
        template<typename T>
        static std::string ToString(const std::vector<T> &t) {
            std::stringstream ss;
            ss << "[";
            for (size_t i = 0; i < t.size(); ++i) {
                ss << ToString(t[i]); // Recursive call to the correct to_string overload!
                if (i < t.size() - 1) ss << ", ";
            }
            ss << "]";
            return ss.str();
        }
    };

    struct DimTraits {
        template<typename T>
        static size_t GetDims(const T &t) {
            return 1; // Default for scalar types
        }

        template<typename T>
        static size_t GetDims(const std::vector<T> &t) {
            return t.size(); // For vector types, return the size of the vector
        }

        template<typename T, size_t N>
        static size_t GetDims(const std::array<T, N> &t) {
            return N; // For array types, return the size of the array
        }

        // Specialization for bool type
        static size_t GetDims(bool) {
            return 1; // bool is treated as a scalar
        }

        // TODO Add more specializations as needed
    };

    template<class T>
    class PropertyArray final : public BasePropertyArray {
    public:
        using ValueType = T;
        using VectorType = std::vector<ValueType>;
        using reference = typename VectorType::reference;
        using const_reference = typename VectorType::const_reference;

        explicit PropertyArray(std::string name, T t = T()) : m_name(std::move(name)), m_value(std::move(t)) {
        }

        void reserve(size_t n) override { m_data.reserve(n); }

        void resize(size_t n) override { m_data.resize(n, m_value); }

        void push_back() override { m_data.push_back(m_value); }

        void free_memory() override { m_data.shrink_to_fit(); }

        void swap(size_t i0, size_t i1) override {
            // Swap elements i0 and i1 in the array this way because std::swap does not work for bool and certain other types ...
            T d(m_data[i0]);
            m_data[i0] = m_data[i1];
            m_data[i1] = d;
        }

        std::unique_ptr<BasePropertyArray> clone() const override {
            auto p = std::make_unique<PropertyArray>(m_name, m_value);
            p->m_data = m_data;
            return p;
        }

        VectorType &vector() { return m_data; }

        const VectorType &vector() const { return m_data; }

        reference operator[](size_t idx) {
            assert(idx < m_data.size());
            return m_data[idx];
        }

        const_reference operator[](size_t idx) const {
            assert(idx < m_data.size());
            return m_data[idx];
        }

        [[nodiscard]] const std::string &name() const override { return m_name; }

        [[nodiscard]] std::string to_string() const override {
            return StringTraits::ToString<>(m_data);
        }

        [[nodiscard]] std::string to_string(size_t i) const override {
            return StringTraits::ToString(m_data[i]);
        }

        [[nodiscard]] size_t size() const override {
            return m_data.size();
        }

        [[nodiscard]] size_t dims() const override {
            return DimTraits::GetDims(m_value);
        }

        [[nodiscard]] const void *data() const override {
            return m_data.empty() ? nullptr : static_cast<const void *>(m_data.data());
        }

        [[nodiscard]] size_t total_size_bytes() override {
            return m_data.size() * sizeof(ValueType);
        }

    private:
        std::string m_name;
        VectorType m_data;
        ValueType m_value;
    };

    template<>
    inline const void *PropertyArray<bool>::data() const {
        assert(false);
        return nullptr;
    }

    class PropertyContainer;

    template<class T>
    class Property {
    public:
        using reference = typename PropertyArray<T>::reference;
        using const_reference = typename PropertyArray<T>::const_reference;

        Property() = default;

        bool is_valid() const { return !m_parray.expired(); }

        operator bool() const { return is_valid(); }

        [[nodiscard]] const std::string &name() const { return m_name; }

        reference operator[](size_t i) {
            auto locked_ptr = m_parray.lock();
            assert(locked_ptr && "Attempt to access an expired property handle.");
            return (*locked_ptr)[i];
        }

        const_reference operator[](size_t i) const {
            auto locked_ptr = m_parray.lock();
            assert(locked_ptr && "Attempt to access an expired property handle.");
            return (*locked_ptr)[i];
        }

        const T *data() const {
            auto locked_ptr = m_parray.lock();
            assert(locked_ptr && "Attempt to access an expired property handle.");
            return locked_ptr->data();
        }

        typename PropertyArray<T>::VectorType &vector() {
            auto locked_ptr = m_parray.lock();
            assert(locked_ptr && "Attempt to access an expired property handle.");
            return locked_ptr->vector();
        }

        const typename PropertyArray<T>::VectorType &vector() const {
            auto locked_ptr = m_parray.lock();
            assert(locked_ptr && "Attempt to access an expired property handle.");
            return locked_ptr->vector();
        }

        [[nodiscard]] const BasePropertyArray *base() const {
            auto locked_ptr = m_parray.lock();
            assert(locked_ptr && "Attempt to access an expired property handle.");
            return locked_ptr.get();
        }

    private:
        friend class PropertyContainer;

        explicit Property(std::weak_ptr<PropertyArray<T> > p, const std::string &name) : m_parray(p), m_name(name) {
        }

        PropertyArray<T> &array() {
            assert(is_valid());
            return *m_parray;
        }

        const PropertyArray<T> &array() const {
            assert(is_valid());
            return *m_parray;
        }

        std::weak_ptr<PropertyArray<T> > m_parray;
        std::string m_name;
    };

    class PropertyContainer {
    public:
        PropertyContainer() = default;

        virtual ~PropertyContainer() { clear(); }

        PropertyContainer(const PropertyContainer &rhs) { operator=(rhs); }

        PropertyContainer &operator=(const PropertyContainer &rhs) {
            if (this != &rhs) {
                clear();
                m_parrays.resize(rhs.n_properties());
                m_size = rhs.size();
                for (size_t i = 0; i < m_parrays.size(); ++i) {
                    m_parrays[i] = std::move(rhs.m_parrays[i]->clone());
                    m_property_map[m_parrays[i]->name()] = i;
                }
            }
            return *this;
        }

        [[nodiscard]] bool empty() const { return m_size == 0; }

        [[nodiscard]] size_t size() const { return m_size; }

        [[nodiscard]] size_t n_properties() const { return m_parrays.size(); }

        [[nodiscard]] std::vector<std::string> properties(const std::initializer_list<int> filter_dims = {}) const {
            //TODO figure out filtering by type, float, int, other custom types ...
            std::vector<std::string> names;
            names.reserve(m_parrays.size());
            for (const auto &parray: m_parrays) {
                if (filter_dims.size() > 0) {
                    for (const auto &dim: filter_dims) {
                        if (parray->dims() == dim) {
                            names.emplace_back(parray->name());
                        }
                    }
                } else {
                    names.emplace_back(parray->name());
                }
            }
            return names;
        }

        template<class T>
        Property<T> add(const std::string &name, const T &t = T()) {
            if (m_property_map.count(name)) {
                throw std::runtime_error("Property '" + name + "' already exists.");
            }

            // Create the array using make_shared.
            auto parray = std::make_shared<PropertyArray<T> >(name, t);
            parray->resize(m_size);

            m_parrays.push_back(parray); // Store the shared_ptr<BasePropertyArray>
            m_property_map[name] = m_parrays.size() - 1;

            // Give the handle a WEAK pointer. Never expose the shared_ptr.
            return Property<T>(parray, name);
        }

        [[nodiscard]] bool exists(const std::string &name) const {
            return m_property_map.find(name) != m_property_map.end();
        }

        template<class T>
        Property<T> get(const std::string &name) {
            auto it = m_property_map.find(name);
            if (it != m_property_map.end()) {
                auto &base_sptr = m_parrays[it->second];

                // Try to downcast the shared_ptr.
                if (auto typed_sptr = std::dynamic_pointer_cast<PropertyArray<T> >(base_sptr)) {
                    // Give the handle a WEAK pointer.
                    return Property<T>(typed_sptr, name);
                }
            }
            return Property<T>();
        }

        BasePropertyArray *get(const std::string &name) const {
            auto it = m_property_map.find(name);
            if (it != m_property_map.end()) {
                return m_parrays[it->second].get();
            }
            return nullptr;
        }

        [[nodiscard]] BasePropertyArray *get_base(const std::string &name) const {
            auto it = m_property_map.find(name);
            if (it != m_property_map.end()) {
                return m_parrays[it->second].get();
            }
            return nullptr;
        }

        template<class T>
        Property<T> get_or_add(const std::string &name, const T t = T()) {
            Property<T> prop = get<T>(name);
            if (!prop) {
                prop = add<T>(name, t);
            }
            return prop;
        }

        void remove(const std::string &name) {
            auto it = m_property_map.find(name);
            if (it == m_property_map.end()) {
                return; // Property doesn't exist
            }

            size_t index_to_remove = it->second;
            size_t last_index = m_parrays.size() - 1;

            // Get the name of the last element before we move it
            std::string last_element_name = m_parrays.back()->name();

            // Swap the element to be removed with the last element
            // This is O(1) for the vector itself.
            m_parrays[index_to_remove] = std::move(m_parrays[last_index]);

            // Pop the now-unused last element
            m_parrays.pop_back();

            // Remove the original property from the map
            m_property_map.erase(it);

            // IMPORTANT: If we removed an element that wasn't the last one,
            // we must update the map index of the element we moved.
            if (index_to_remove != last_index) {
                m_property_map[last_element_name] = index_to_remove;
            }
        }

        template<class T>
        void remove(Property<T> &prop) {
            remove(prop.name());
        }

        void clear() {
            m_parrays.clear();
            m_size = 0;
        }

        void reserve(size_t n) {
            for (const auto &parray: m_parrays) {
                parray->reserve(n);
            }
        }

        void resize(size_t n) {
            for (const auto &parray: m_parrays) {
                parray->resize(n);
            }
            m_size = n;
        }

        void free_memory() {
            for (const auto &parray: m_parrays) {
                parray->free_memory();
            }
        }

        void push_back() {
            for (const auto &parray: m_parrays) {
                parray->push_back();
            }
            ++m_size;
        }

        void swap(size_t i0, size_t i1) {
            for (const auto &parray: m_parrays) {
                parray->swap(i0, i1);
            }
        }

    private:
        std::vector<std::shared_ptr<BasePropertyArray> > m_parrays;
        std::unordered_map<std::string, size_t> m_property_map;
        size_t m_size{0};
    };
}
