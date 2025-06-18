// RDE_Project/modules/core/include/Core/Log.h

#pragma once // Modern alternative to include guards.

#include "Core/Base.h" // Include base definitions

#include <memory>

// This is our abstraction. We are including the spdlog header here,
// but the rest of our engine will only include this Log.h file.
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

class Log
{
public:
    static void Initialize();

    // In a real application, you might want to remove these getters and
    // force all logging through macros to add file/line info automatically.
    static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_core_logger; }
    static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_client_logger; }

private:
    static std::shared_ptr<spdlog::logger> s_core_logger;
    static std::shared_ptr<spdlog::logger> s_client_logger;
};

// --- Core Logging Macros ---
#define RDE_CORE_TRACE(...)    ::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define RDE_CORE_INFO(...)     ::Log::GetCoreLogger()->info(__VA_ARGS__)
#define RDE_CORE_WARN(...)     ::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define RDE_CORE_ERROR(...)    ::Log::GetCoreLogger()->error(__VA_ARGS__)
#define RDE_CORE_CRITICAL(...) ::Log::GetCoreLogger()->critical(__VA_ARGS__)

// --- Client Logging Macros ---
#define RDE_TRACE(...)         ::Log::GetClientLogger()->trace(__VA_ARGS__)
#define RDE_INFO(...)          ::Log::GetClientLogger()->info(__VA_ARGS__)
#define RDE_WARN(...)          ::Log::GetClientLogger()->warn(__VA_ARGS__)
#define RDE_ERROR(...)         ::Log::GetClientLogger()->error(__VA_ARGS__)
#define RDE_CRITICAL(...)      ::Log::GetClientLogger()->critical(__VA_ARGS__)