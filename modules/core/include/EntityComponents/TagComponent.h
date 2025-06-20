// RDE_Project/modules/core/include/Components/TagComponent.h
#pragma once

#include <string>

namespace RDE {
    struct TagComponent {
        std::string tag;

        TagComponent() = default;

        TagComponent(const TagComponent &) = default;

        TagComponent(const std::string &tag) : tag(tag) {}
    };
}