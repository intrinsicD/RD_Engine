// RDE_Project/modules/core/include/Log.h

#pragma once // Modern alternative to include guards.

#include "Base.h"

#include <memory>

// This is our abstraction. We are including the spdlog header here,
// but the rest of our engine will only include this Log.h file.
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h" // Important for ostream operator<<
#include "spdlog/fmt/fmt.h"   // Include the core fmt library header for FMT_STRING

namespace RDE {
    class Log {
    public:
        static void Initialize();

        // In a real application, you might want to remove these getters and
        // force all logging through macros to add file/line info automatically.
        static std::shared_ptr<spdlog::logger> &GetCoreLogger() { return s_core_logger; }

        static std::shared_ptr<spdlog::logger> &GetClientLogger() { return s_client_logger; }

    private:
        static std::shared_ptr<spdlog::logger> s_core_logger;
        static std::shared_ptr<spdlog::logger> s_client_logger;
    };

// --- Core Logging Macros ---
#define RDE_CORE_TRACE(...)    RDE::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define RDE_CORE_INFO(...)     RDE::Log::GetCoreLogger()->info(__VA_ARGS__)
#define RDE_CORE_WARN(...)     RDE::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define RDE_CORE_ERROR(...)    RDE::Log::GetCoreLogger()->error(__VA_ARGS__)
#define RDE_CORE_CRITICAL(...) RDE::Log::GetCoreLogger()->critical(__VA_ARGS__)

// --- Client Logging Macros ---
#define RDE_TRACE(...)         RDE::Log::GetClientLogger()->trace(__VA_ARGS__)
#define RDE_INFO(...)          RDE::Log::GetClientLogger()->info(__VA_ARGS__)
#define RDE_WARN(...)          RDE::Log::GetClientLogger()->warn(__VA_ARGS__)
#define RDE_ERROR(...)         RDE::Log::GetClientLogger()->error(__VA_ARGS__)
#define RDE_CRITICAL(...)      RDE::Log::GetClientLogger()->critical(__VA_ARGS__)
}