# RDE_Project/CMakeLists.txt

cmake_minimum_required(VERSION 3.22)
project(RD_Engine VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
# --- Third-Party Dependencies ---
include(cmake/Dependencies.cmake)

string(REPLACE "\\" "/" PROJECT_SOURCE_DIR_SLASH_FORWARD ${PROJECT_SOURCE_DIR})
add_compile_definitions(RDE_PROJECT_ROOT_DIR="${PROJECT_SOURCE_DIR_SLASH_FORWARD}")
add_compile_options("-Wall" "-Wextra" "-Werror")

# --- Project Structure ---
add_subdirectory(modules)
add_subdirectory(applications)