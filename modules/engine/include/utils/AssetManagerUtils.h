#pragma once

#include "assets/AssetManager.h"
#include "IRenderer.h"

namespace RDE{
    std::shared_ptr<GeometryData> load_obj(const std::string &path);
    std::shared_ptr<GeometryData> load_gltf(const std::string &path);
    std::shared_ptr<GeometryData> load_stl(const std::string &path);
    std::shared_ptr<GeometryData> load_off(const std::string &path);
    std::shared_ptr<GeometryData> load_ply(const std::string &path);
}