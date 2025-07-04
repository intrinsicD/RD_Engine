include(FetchContent)

# -----------------------------------------------------------------------------
# 1. DECLARE ALL DEPENDENCIES
#
# We declare all dependencies upfront. This allows CMake to de-duplicate
# requests if libraries share common dependencies.
# -----------------------------------------------------------------------------

message(STATUS "Fetching third-party dependencies...")

FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.14.1)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.90.8-docking)
FetchContent_Declare(ImGuiFileDialog GIT_REPOSITORY https://github.com/aiekick/ImGuiFileDialog.git GIT_TAG v0.6.7)
FetchContent_Declare(glad GIT_REPOSITORY https://github.com/Dav1dde/glad.git GIT_TAG v2.0.4 SOURCE_SUBDIR cmake)
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1)
FetchContent_Declare(stb_image GIT_REPOSITORY https://github.com/nothings/stb.git GIT_TAG master)
FetchContent_Declare(EnTT GIT_REPOSITORY https://github.com/skypjack/entt.git GIT_TAG v3.15.0)
FetchContent_Declare(tinyobjloader GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git GIT_TAG v2.0.0rc13)
FetchContent_Declare(yaml-cpp GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git GIT_TAG master)
FetchContent_Declare(efsw GIT_REPOSITORY https://github.com/SpartanJ/efsw.git GIT_TAG 1.4.1)
FetchContent_Declare(VulkanMemoryAllocator GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git GIT_TAG v3.0.1)
FetchContent_Declare(daxa GIT_REPOSITORY https://github.com/Ipotrick/Daxa GIT_TAG 3.1)

# -----------------------------------------------------------------------------
# 2. CONFIGURE & POPULATE DEPENDENCIES
#
# Before calling FetchContent_MakeAvailable, we can set options to control
# how the dependencies are built. This prevents building unnecessary examples,
# tests, or docs, speeding up our configuration and build times.
# -----------------------------------------------------------------------------

# --- GLFW ---
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# --- GLAD ---
# --- Isolate GLAD to handle its legacy CMake script ---
# We PUSH the policy stack to create a safe, temporary scope.
cmake_policy(PUSH)
# We set a broad legacy policy version just for glad.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_POLICY_DEFAULT_CMP0148 OLD) # This avoids the deprecation warning for legacy CMake scripts.
# We call FetchContent_MakeAvailable *inside* the policy block.
# This is the modern approach that avoids the deprecation warning AND
# correctly handles the SOURCE_SUBDIR parameter from our declaration.
message(STATUS "Handling GLAD with custom policy scope...")
FetchContent_MakeAvailable(glad)
# We POP the policy stack, restoring our project's modern settings.
cmake_policy(POP)
# We will link against glad::gl_core
glad_add_library(glad_gl_core STATIC REPRODUCIBLE API gl:core=4.6) # Using 4.6 as a modern default

# --- ImGui ---
# Disable building tests/examples that we don't need.
set(IMGUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(IMGUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(imgui)

# --- All other dependencies ---
# Most of these are header-only or have simple builds.
FetchContent_MakeAvailable(VulkanMemoryAllocator)
#FetchContent_MakeAvailable(daxa)
FetchContent_MakeAvailable(spdlog ImGuiFileDialog EnTT tinyobjloader yaml-cpp stb_image efsw glm)