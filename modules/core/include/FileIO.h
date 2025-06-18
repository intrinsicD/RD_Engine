// RDE_Project/modules/core/include/Core/FileIO.h
#pragma once

#include <string>
#include "Log.h"

namespace RDE {
    class FileIO {
    public:
        static std::string read_file(const std::string &filepath);

        static std::string get_path(const std::string &relativePath);
    };
}