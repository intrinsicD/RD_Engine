// RDE_Project/modules/core/include/Base.h

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

// This macro simplifies binding a class member function to a std::function.
// It takes a member function (fn) and creates a callable that takes one argument,
// automatically binding 'this' as the object instance.
#define RDE_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define RDE_MOUSE_BUTTON_LEFT      0
#define RDE_MOUSE_BUTTON_RIGHT     1
#define RDE_MOUSE_BUTTON_MIDDLE    2
#define RDE_MOUSE_BUTTON_4         3
#define RDE_MOUSE_BUTTON_5         4
#define RDE_MOUSE_BUTTON_6         5
#define RDE_MOUSE_BUTTON_7         6
#define RDE_MOUSE_BUTTON_8         7
#define RDE_MOUSE_BUTTON_LAST      RDE_MOUSE_BUTTON_8


