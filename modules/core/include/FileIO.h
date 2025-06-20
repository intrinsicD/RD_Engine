// RDE_Project/modules/core/include/FileIO.h
#pragma once

#include <string>
#include "Log.h"

namespace RDE {
    class FileIO {
    public:
        static std::string read_file(const std::string &filepath);

        static std::string GetPath(const std::string &relativePath);
    };
}