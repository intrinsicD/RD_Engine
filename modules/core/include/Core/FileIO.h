// RDE_Project/modules/core/include/Core/FileIO.h
#pragma once

#include <string>
#include "Core/Log.h"

class FileIO {
public:
    static std::string ReadFile(const std::string &filepath);

    static std::string GetPath(const std::string &relativePath);
};