//
// Created by alex on 18.06.25.
//

#include "FileIO.h"
#include "Log.h"

#include <fstream>
#include <sstream>

namespace RDE {
    std::string FileIO::read_file(const std::string &filepath) {
        std::ifstream file(filepath);
        if (!file) {
            RDE_CORE_ERROR("Could not open file: '{0}'", filepath);
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    std::string FileIO::get_path(const std::string &relativePath) {
        // RDE_PROJECT_ROOT_DIR is defined by CMake.
        return std::string(RDE_PROJECT_ROOT_DIR) + "/" + relativePath;
    }
}
