# RDE_Project/modules/engine/CMakeLists.txt

set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# An INTERFACE library has no source files and doesn't produce a binary.
# It's used to group together usage requirements like include directories.
add_library(Engine)
add_library(RDE::Engine ALIAS Engine)

# Anything that links to this library will automatically get its include path.
target_include_directories(Engine
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${glm_SOURCE_DIR}
        ${imgui_SOURCE_DIR}
)

target_sources(Engine
        PRIVATE
        src/layers/ImGuiLayer.cpp
        src/systems/InputSystem.cpp
        src/systems/TransformSystem.cpp
        src/Application.cpp
        src/FileIOUtils.cpp
)

target_link_libraries(Engine
        PUBLIC
        EnTT
        spdlog
        RDE::Platform::Glfw
        RDE::Platform::OpenGL
        yaml-cpp
)