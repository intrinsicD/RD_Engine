#pragma once

#include <filesystem>

namespace RDE::FileIO {
    std::string ReadFile(const std::filesystem::path &path);

    bool WriteFile(const std::filesystem::path &path, const std::string &content);
}
