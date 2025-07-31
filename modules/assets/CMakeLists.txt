# modules/assets/CMakeLists.txt


# An INTERFACE library has no source files and doesn't produce a binary.
# It's used to group together usage requirements like include directories.
add_library(AssetSystem)
add_library(RDE::AssetSystem ALIAS AssetSystem)

# Anything that links to this library will automatically get its include path.
target_include_directories(AssetSystem
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(AssetSystem
        PRIVATE
        src/FileWatcher.cpp
        src/MeshObjLoader.cpp
        src/MeshMtlLoader.cpp
        src/MaterialManifestLoader.cpp
        src/StbImageLoader.cpp
        src/ShaderDefLoader.cpp
)

target_link_libraries(AssetSystem
        PUBLIC
        RDE::Core
        RDE::Renderer
        RDE::StbImage
        EnTT::EnTT
        efsw
        tinyobjloader
        yaml-cpp::yaml-cpp
)

add_subdirectory(config)