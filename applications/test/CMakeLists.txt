# RDE_Project/applications/sandbox/CMakeLists.txt

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

add_executable(Test)

target_include_directories(Test PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${imgui_SOURCE_DIR}
)

target_sources(Test PRIVATE
        Application.cpp
        Platform.cpp
        ${IMGUI_SOURCES}
)

find_package(OpenGL REQUIRED)
target_link_libraries(Test
        PRIVATE
        RDE::AssetSystem
        RDE::Core
        EnTT
        spdlog
        glfw
        glm
        glad_gl_core
        OpenGL::GL
)

add_subdirectory(assets)