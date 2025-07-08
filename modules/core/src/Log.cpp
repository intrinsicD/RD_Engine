#include "core/Log.h"

namespace RDE {
    // Definition for static members
    std::shared_ptr<spdlog::logger> Log::s_core_logger;
    std::shared_ptr<spdlog::logger> Log::s_client_logger;

    void Log::Initialize() {
        // Configure the logger pattern.
        // [%T] - Timestamp, %n - Logger's name, %v%$ - The message
        spdlog::set_pattern("%^[%T] %n: %v%$");

        s_core_logger = spdlog::stdout_color_mt("ENGINE");
        s_core_logger->set_level(spdlog::level::trace);

        s_client_logger = spdlog::stdout_color_mt("APP");
        s_client_logger->set_level(spdlog::level::trace);

        // Initial log messages to confirm setup
        RDE_CORE_WARN("Initialized Core Logger!");
        RDE_INFO("Initialized Client Logger!");
    }
}