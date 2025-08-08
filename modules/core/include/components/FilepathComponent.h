#pragma once

#include <filesystem>

namespace RDE {
    struct FilepathComponent {
        std::filesystem::path filepath; // The file path associated with this component
        std::string filename; // The name of the file without the path
        std::string extension; // The file extension (e.g., .txt, .png)
    };

    inline std::string GetAbsolutePath(const std::filesystem::path &filepath, const std::string &filename,
                                       const std::string &extension) {
        // This function returns the absolute path of the file.
        return filepath / (filename + extension);
    }

    inline FilepathComponent GetFilepathComponent(const std::filesystem::path &fullpath) {
        FilepathComponent component;
        component.filepath = fullpath.parent_path();
        component.filename = fullpath.stem().string(); // Get the filename without extension
        component.extension = fullpath.extension().string(); // Get the file extension
        return component;
    }
}
