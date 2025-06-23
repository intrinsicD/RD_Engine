// file: modules/common/include/Common/HashUtils.h
#pragma once

#include <glm/glm.hpp>
#include <cstddef> // For size_t

namespace RDE {
    /**
     * @brief A robust hash combination function, inspired by boost::hash_combine.
     * @tparam T The type of the value to combine into the seed.
     * @param seed The current accumulated hash value (in/out).
     * @param value The new value to hash and combine.
     */
    template<class T>
    inline void HashCombine(std::size_t &seed, const T &value) {
        std::hash < T > hasher;
        seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}

// Now, provide the hash specializations for third-party types (glm) here.
// Because this is a header file in 'common' and will be included by many
// modules, it's the correct central place for these general utilities.
namespace std {
    template<>
    struct hash<glm::vec2> {
        size_t operator()(const glm::vec2 &v) const {
            size_t seed = 0;
            RDE::HashCombine(seed, v.x);
            RDE::HashCombine(seed, v.y);
            return seed;
        }
    };

    template<>
    struct hash<glm::vec3> {
        size_t operator()(const glm::vec3 &v) const {
            size_t seed = 0;
            RDE::HashCombine(seed, v.x);
            RDE::HashCombine(seed, v.y);
            RDE::HashCombine(seed, v.z);
            return seed;
        }
    };

    template<>
    struct hash<glm::vec4> {
        size_t operator()(const glm::vec4 &v) const {
            size_t seed = 0;
            RDE::HashCombine(seed, v.x);
            RDE::HashCombine(seed, v.y);
            RDE::HashCombine(seed, v.z);
            RDE::HashCombine(seed, v.w);
            return seed;
        }
    };
}
