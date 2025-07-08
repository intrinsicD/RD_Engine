#pragma once

#include <filesystem>
#include <vector>

namespace RDE::FileIO {
    std::string GetFileExtension(const std::filesystem::path &path);

    std::filesystem::path GetFileName(const std::filesystem::path &path);

    std::vector<char> ReadFile(const std::filesystem::path &path);

    bool WriteFile(const std::filesystem::path &path, const std::string &content);
}
