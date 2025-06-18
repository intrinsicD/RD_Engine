//
// Created by alex on 18.06.25.
//

#include "Core/FileIO.h"
#include "Core/Log.h"
#include <fstream>
#include <sstream>

std::string FileIO::ReadFile(const std::string &filepath){
    std::ifstream file(filepath);
    if (!file)
    {
        RDE_CORE_ERROR("Could not open file: '{0}'", filepath);
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string FileIO::GetPath(const std::string& relativePath)
{
    // RDE_PROJECT_ROOT_DIR is defined by CMake.
    return std::string(RDE_PROJECT_ROOT_DIR) + "/" + relativePath;
}