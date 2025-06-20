#include "ComponentUIRegistry.h"

namespace RDE {
    std::unordered_map<std::type_index, ComponentUIFunctions> ComponentUIRegistry::s_component_ui_map;
}
