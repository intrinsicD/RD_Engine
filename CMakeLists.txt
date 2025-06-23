# RDE_Project/CMakeLists.txt

cmake_minimum_required(VERSION 3.22)
project(RD_Engine VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Third-Party Dependencies ---
include(FetchContent)

# Declare all dependencies.
FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.14.1)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.90.8-docking)
FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG        v2.0.4              # or whichever version you want
        SOURCE_SUBDIR  cmake
)

FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 0.9.9.8)
FetchContent_Declare(stb GIT_REPOSITORY https://github.com/nothings/stb.git GIT_TAG master)
FetchContent_Declare(EnTT GIT_REPOSITORY https://github.com/skypjack/entt.git GIT_TAG v3.13.0)
FetchContent_Declare(tinyobjloader GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git GIT_TAG v2.0.0rc13)
FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG 0.8.0 # Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
)

# Make them all available. This command triggers the download and runs each dependency's
# own CMakeLists.txt, preparing their targets for us to use.
FetchContent_MakeAvailable(spdlog glfw imgui stb EnTT tinyobjloader)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_MakeAvailable(glad glm yaml-cpp)
glad_add_library(glad_gl_core STATIC REPRODUCIBLE API gl:core=4.3)

string(REPLACE "\\" "/" PROJECT_SOURCE_DIR_SLASH_FORWARD ${PROJECT_SOURCE_DIR})
add_compile_definitions(RDE_PROJECT_ROOT_DIR="${PROJECT_SOURCE_DIR_SLASH_FORWARD}")

# --- Project Structure ---
add_subdirectory(modules)
add_subdirectory(applications)