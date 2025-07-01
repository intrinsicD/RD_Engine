#pragma once

#include <filesystem>
#include <string>
#include <cstdint>
#include <optional>

// The RDE::Platform namespace is the single entry point for all OS-specific functionality.
namespace RDE::Platform {

    /**
     * @brief Retrieves the full path to the currently running executable.
     * @return An optional path to the executable. Returns std::nullopt on failure.
     */
    std::optional<std::filesystem::path> get_executable_path();

    /**
     * @brief Retrieves the current value of a high-resolution performance counter.
     * The unit (e.g., nanoseconds, ticks) is platform-dependent; use get_performance_frequency()
     * to convert this to seconds.
     * @return The raw performance counter value.
     */
    uint64_t get_performance_counter();

    /**
     * @brief Retrieves the frequency of the high-resolution performance counter (ticks per second).
     * @return The number of ticks per second.
     */
    uint64_t get_performance_frequency();
    
    /**
     * @brief Loads a dynamic link library (DLL, .so, .dylib).
     * @param library_path The path to the library file.
     * @return A handle to the loaded library, or nullptr on failure.
     */
    void* load_dynamic_library(const std::filesystem::path& library_path);
    
    /**
     * @brief Frees a previously loaded dynamic library.
     * @param library_handle The handle returned by load_dynamic_library.
     */
    void free_dynamic_library(void* library_handle);
    
    /**
     * @brief Retrieves a function pointer from a loaded dynamic library.
     * @param library_handle The handle to the library.
     * @param function_name The name of the function to retrieve.
     * @return A pointer to the function, or nullptr if not found.
     */
    void* get_function_pointer(void* library_handle, const std::string& function_name);

    // Add other platform functions as needed:
    // - Displaying a system message box
    // - Getting the number of logical CPU cores
    // - Getting system memory information
}