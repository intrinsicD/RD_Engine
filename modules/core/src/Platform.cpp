#include "core/Platform.h"

// Platform-specific headers
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#elif defined(__linux__)
#include <unistd.h>
#include <dlfcn.h>
#include <climits> // For PATH_MAX
#include <time.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <dlfcn.h>
#endif

namespace RDE::Platform {

    std::optional<std::filesystem::path> get_executable_path() {
#ifdef _WIN32
        // Windows uses wide characters for paths
        wchar_t path_buffer[MAX_PATH] = {0};
        // GetModuleFileNameW(NULL, ...) gets the path of the current process's executable
        if (GetModuleFileNameW(NULL, path_buffer, MAX_PATH) == 0) {
            return std::nullopt; // Failure
        }
        return std::filesystem::path(path_buffer);
#elif defined(__linux__)
        char result[PATH_MAX];
        // On Linux, the executable path is a symbolic link at /proc/self/exe
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        if (count == -1) {
            return std::nullopt; // Failure
        }
        return std::string(result, count);
#elif defined(__APPLE__)
        char path_buffer[1024];
        uint32_t size = sizeof(path_buffer);
        // Apple provides a dedicated function for this
        if (_NSGetExecutablePath(path_buffer, &size) != 0) {
            return std::nullopt; // Failure (buffer was too small, could reallocate and retry)
        }
        return std::filesystem::path(path_buffer);
#else
        // If the platform is not recognized, return failure.
        #warning "get_executable_path() is not implemented for this platform."
        return std::nullopt;
#endif
    }

    uint64_t get_performance_counter() {
#ifdef _WIN32
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        return counter.QuadPart;
#elif defined(__linux__) || defined(__APPLE__)
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return static_cast<uint64_t>(ts.tv_sec) * 1000000000ULL + ts.tv_nsec;
#endif
    }

    uint64_t get_performance_frequency() {
#ifdef _WIN32
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        return frequency.QuadPart;
#elif defined(__linux__) || defined(__APPLE__)
        // Our get_performance_counter implementation for these platforms returns nanoseconds directly.
        // Therefore, the frequency is 1 billion ticks per second.
        return 1000000000ULL;
#endif
    }
    
    void* load_dynamic_library(const std::filesystem::path& library_path) {
#ifdef _WIN32
        return LoadLibraryW(library_path.c_str());
#else // Linux and macOS use dlopen
        return dlopen(library_path.c_str(), RTLD_NOW);
#endif
    }

    void free_dynamic_library(void* library_handle) {
        if (!library_handle) return;
#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(library_handle));
#else
        dlclose(library_handle);
#endif
    }

    void* get_function_pointer(void* library_handle, const std::string& function_name) {
        if (!library_handle) return nullptr;
#ifdef _WIN32
        return GetProcAddress(static_cast<HMODULE>(library_handle), function_name.c_str());
#else
        return dlsym(library_handle, function_name.c_str());
#endif
    }

} // namespace RDE::Platform