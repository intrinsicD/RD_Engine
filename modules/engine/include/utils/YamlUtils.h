#pragma once

#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

namespace YAML {
    // This allows us to emit glm vectors directly into the YAML stream
    inline Emitter &operator<<(Emitter &out, const glm::vec3 &v) {
        out << Flow << BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    inline Emitter &operator<<(Emitter &out, const glm::vec4 &v) {
        out << Flow << BeginSeq << v.r << v.g << v.b << v.a << YAML::EndSeq;
        return out;
    }
}
// Template specialization to teach yaml-cpp how to handle glm::vec3
template<>
struct YAML::convert<glm::vec3> {
    static Node encode(const glm::vec3 &rhs) {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        node.SetStyle(EmitterStyle::Flow); // [x, y, z]
        return node;
    }

    static bool decode(const Node &node, glm::vec3 &rhs) {
        if (!node.IsSequence() || node.size() != 3) return false;
        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        return true;
    }
};

// And for glm::vec4
template<>
struct YAML::convert<glm::vec4> {
    static Node encode(const glm::vec4 &rhs) {
        Node node;
        node.push_back(rhs.r);
        node.push_back(rhs.g);
        node.push_back(rhs.b);
        node.push_back(rhs.a);
        node.SetStyle(EmitterStyle::Flow); // [r, g, b, a]
        return node;
    }

    static bool decode(const Node &node, glm::vec4 &rhs) {
        if (!node.IsSequence() || node.size() != 4) return false;
        rhs.r = node[0].as<float>();
        rhs.g = node[1].as<float>();
        rhs.b = node[2].as<float>();
        rhs.a = node[3].as<float>();
        return true;
    }
};