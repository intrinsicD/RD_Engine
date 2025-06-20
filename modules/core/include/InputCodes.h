#pragma once

// These codes are an abstraction. They can be mapped to any backend (GLFW, Win32, etc.).
// We use #define here for simplicity, matching the style of libraries like GLFW.
// Other valid approaches include a namespace with constexpr ints.

// From glfw3.h
namespace RDE {
#define RDE_MOUSE_BUTTON_LEFT      0
#define RDE_MOUSE_BUTTON_RIGHT     1
#define RDE_MOUSE_BUTTON_MIDDLE    2
#define RDE_MOUSE_BUTTON_4         3
#define RDE_MOUSE_BUTTON_5         4
#define RDE_MOUSE_BUTTON_6         5
#define RDE_MOUSE_BUTTON_7         6
#define RDE_MOUSE_BUTTON_8         7
#define RDE_MOUSE_BUTTON_LAST      RDE_MOUSE_BUTTON_8
}