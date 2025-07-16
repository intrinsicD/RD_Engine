#include "core/FileIOUtils.h"
#include "core/Log.h"

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

    std::vector<char> ReadFile(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            RDE_CORE_ERROR("Failed to open file: {}", path.string());
            return {};
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        if (buffer.empty()) {
            RDE_CORE_WARN("File is empty: {}", path.string());
        } else {
            RDE_CORE_TRACE("Successfully read file: {}", path.string());
        }
        return buffer;
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
        RDE_CORE_TRACE("Successfully wrote to file: {}", path.string());
        return true;
    }
}
