# RDE_Project/modules/engine/CMakeLists.txt

set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${ImGuiFileDialog_SOURCE_DIR}/ImGuiFileDialog.cpp
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
        ${ImGuiFileDialog_SOURCE_DIR}
)

target_sources(Engine
        PRIVATE
        src/layers/ImGuiLayer.cpp
        src/Engine.cpp
        src/utils/FileIOUtils.cpp
        src/Scene.cpp
        src/InputManager.cpp
        ${IMGUI_SOURCES}
)

target_link_libraries(Engine
        PUBLIC
        EnTT::EnTT
        spdlog
        RDE::AssetSystem
        RDE::Renderer
        RDE::Core
        RDE::Platform::Glfw
        yaml-cpp
        tinyobjloader
)