# applications/sandbox/CMakeLists.txt

add_library(rde_imgui STATIC)
add_library(RDE::ImGui ALIAS rde_imgui)
target_sources(rde_imgui PRIVATE
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
        ${ImGuiFileDialog_SOURCE_DIR}/ImGuiFileDialog.cpp
)
target_compile_options(rde_imgui PRIVATE
        "-Wno-unused-parameter"         # The one we had before
        "-Wno-overloaded-virtual"       # FIX for the what() error
        "-Wno-deprecated-enum-enum-conversion" # FIX for the bitwise enum error
)
target_include_directories(rde_imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${ImGuiFileDialog_SOURCE_DIR}
)
target_link_libraries(rde_imgui PRIVATE glfw)

add_executable(Test)

target_include_directories(Test PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(Test PRIVATE
        SandboxApp.cpp
        main.cpp
        ImGuiLayer.cpp
        TestSceneLayer.cpp
        AssetViewerLayer.cpp
)

find_package(OpenGL REQUIRED)

target_link_libraries(Test
        PRIVATE
        RDE::AssetSystem
        RDE::Core
        RDE::Platform::Vulkan
        RDE::Platform::Glfw
        RDE::Scene
        RDE::Geometry
        RDE::Renderer
        RDE::ImGui
        spdlog
        glfw
        glm
        glad_gl_core
        OpenGL::GL
        tinyobjloader
        yaml-cpp
)