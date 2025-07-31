#modules/scene/CMakeLists.txt

# An INTERFACE library has no source files and doesn't produce a binary.
# It's used to group together usage requirements like include directories.
add_library(Scene)
add_library(RDE::Scene ALIAS Scene)

# Anything that links to this library will automatically get its include path.
target_include_directories(Scene
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(Scene
        PRIVATE
        src/Scene.cpp

        src/BoundingVolumeComponent.cpp
        src/CameraComponent.cpp
        src/HierarchyComponent.cpp
        src/TransformComponent.cpp

        src/CameraSystem.cpp
        src/TransformSystem.cpp
        src/HierarchySystem.cpp
        src/BoundingVolumeSystem.cpp
        src/RenderSystem.cpp
        src/RenderPacketSystem.cpp
)

target_link_libraries(Scene
        PUBLIC
        RDE::Core
        RDE::AssetSystem
        RDE::Geometry
        EnTT::EnTT
)