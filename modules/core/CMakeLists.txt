

# An INTERFACE library has no source files and doesn't produce a binary.
# It's used to group together usage requirements like include directories.
add_library(Core)
add_library(RDE::Core ALIAS Core)

# Anything that links to this library will automatically get its include path.
target_include_directories(Core
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(Core
        PRIVATE
        src/Log.cpp
)

target_link_libraries(Core
        PUBLIC
        spdlog
)