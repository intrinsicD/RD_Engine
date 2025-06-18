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

# Make them all available. This command triggers the download and runs each dependency's
# own CMakeLists.txt, preparing their targets for us to use.
FetchContent_MakeAvailable(spdlog glfw imgui)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_MakeAvailable(glad)
glad_add_library(glad_gl_core STATIC
        API gl:core=4.3              # adjust your OpenGL version here
        REPRODUCIBLE
)

# --- Project Structure ---
add_subdirectory(modules)
add_subdirectory(applications)