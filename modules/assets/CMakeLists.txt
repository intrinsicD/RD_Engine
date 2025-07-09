# RDE_Project/modules/assets/CMakeLists.txt


# An INTERFACE library has no source files and doesn't produce a binary.
# It's used to group together usage requirements like include directories.
add_library(AssetSystem)
add_library(RDE::AssetSystem ALIAS AssetSystem)

# Anything that links to this library will automatically get its include path.
target_include_directories(AssetSystem
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${stb_image_SOURCE_DIR}
)

target_sources(AssetSystem
        PRIVATE
        src/AssetManager.cpp
        src/FileWatcher.cpp
)

target_link_libraries(AssetSystem
        PUBLIC
        RDE::Core
        RDE::Renderer
        EnTT::EnTT
        efsw
)

add_subdirectory(config)