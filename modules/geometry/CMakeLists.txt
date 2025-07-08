

# An INTERFACE library has no source files and doesn't produce a binary.
# It's used to group together usage requirements like include directories.
add_library(Geometry INTERFACE)
add_library(RDE::Geometry ALIAS Geometry)

# Anything that links to this library will automatically get its include path.
target_include_directories(Geometry
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Geometry
        INTERFACE
        glm::glm
)