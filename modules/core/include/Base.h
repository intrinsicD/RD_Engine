// RDE_Project/modules/core/include/Core/Base.h

#pragma once

#include <cstdint> // For fixed-width integers

// --- Platform Detection ---
#if defined(_WIN32)
    #define RDE_PLATFORM_WINDOWS
#elif defined(__APPLE__) || defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR == 1 || TARGET_OS_IPHONE == 1
        #error "iOS is not supported!"
    #elif TARGET_OS_MAC == 1
        #define RDE_PLATFORM_MACOS
    #else
        #error "Unknown Apple platform!"
    #endif
#elif defined(__linux__)
    #define RDE_PLATFORM_LINUX
#else
    #error "Unknown platform!"
#endif

// --- Debug Break ---
#if defined(RDE_PLATFORM_WINDOWS)
    #define RDE_DEBUG_BREAK() __debugbreak()
#elif defined(RDE_PLATFORM_LINUX) || defined(RDE_PLATFORM_MACOS)
    #include <signal.h>
    #define RDE_DEBUG_BREAK() raise(SIGTRAP)
#else
    #define RDE_DEBUG_BREAK() // Or some other fallback
#endif


// --- Assertions ---
#ifdef RDE_ENABLE_ASSERTS
    // Core assertion implementation
    #define RDE_CORE_ASSERT(check, ...) \
    do { \
    if (!(check)) { \
    RDE_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); \
    RDE_DEBUG_BREAK(); \
    } \
    } while(false)

    // Client assertion implementation
    #define RDE_ASSERT(check, ...) \
    do { \
    if (!(check)) { \
    RDE_ERROR("Assertion Failed: {0}", __VA_ARGS__); \
    RDE_DEBUG_BREAK(); \
    } \
    } while(false)
#else
    #define RDE_ASSERT(check, ...)
    #define RDE_CORE_ASSERT(check, ...)
#endif

