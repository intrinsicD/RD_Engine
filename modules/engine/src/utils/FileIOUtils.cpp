#include "utils/FileIOUtils.h"
#include "Log.h"

#include <fstream>
#include <filesystem>

namespace RDE::FileIO {
    std::string GetFileExtension(const std::filesystem::path &path) {
        if (path.has_extension()) {
            return path.extension().string();
        }
        RDE_CORE_WARN("File has no extension: {}", path.string());
        return {};
    }

    std::filesystem::path GetFileName(const std::filesystem::path &path) {
        if (path.has_filename()) {
            return path.filename();
        }
        RDE_CORE_WARN("Path has no filename: {}", path.string());
        return {};
    }

    std::string ReadFile(const std::filesystem::path &path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            RDE_CORE_ERROR("Failed to open file: {}", path.string());
            return {};
        }
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        if (content.empty()) {
            RDE_CORE_WARN("File is empty: {}", path.string());
        } else {
            RDE_CORE_INFO("Successfully read file: {}", path.string());
        }
        return content;
    }

    bool WriteFile(const std::filesystem::path &path, const std::string &content) {
        std::ofstream file(path);
        if (!file.is_open()) {
            RDE_CORE_ERROR("Failed to open file for writing: {}", path.string());
            return false;
        }
        file << content;
        file.close();
        if (file.fail()) {
            RDE_CORE_ERROR("Failed to write to file: {}", path.string());
            return false;
        }
        RDE_CORE_INFO("Successfully wrote to file: {}", path.string());
        return true;
    }
}
