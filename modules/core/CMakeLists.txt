# RDE_Project/modules/core/CMakeLists.txt

# We need to build the specific backend files for GLFW and OpenGL3.
set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)


# Create a library target for our Core module.
# We will use a namespaced alias 'RDE::Core' for clarity and to avoid name collisions.
# Add Application.cpp to the list of source files for the Core library.
add_library(Core
        src/Application.cpp
        src/Entity.cpp
        src/ImGuiLayer.cpp
        src/Layer.cpp
        src/LayerStack.cpp
        src/Scene.cpp
        src/SceneSerializer.cpp
        ${IMGUI_SOURCES}
)
add_library(RDE::Core ALIAS Core)

# Specify the public include directory for the Core library.
# PUBLIC: Consumers of Core need this path to find its headers.
target_include_directories(Core
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${imgui_SOURCE_DIR}
        ${glad_SOURCE_DIR}/include
        ${glm_SOURCE_DIR}
)

# Add compile definitions specifically to the Core target.
# INTERFACE: Any target that links against Core will inherit these definitions.
# This ensures RDE_ENABLE_ASSERTS is defined project-wide for Debug builds.
target_compile_definitions(Core
        INTERFACE
        $<$<CONFIG:Debug>:RDE_ENABLE_ASSERTS>
)

find_package(OpenGL REQUIRED)
target_link_libraries(Core
        PUBLIC
        glfw
        glad_gl_core  # Use the glad target provided by FetchContent
        OpenGL::GL
        EnTT::EnTT
        yaml-cpp
        RDE::Common
        RDE::Assets
)

get_target_property(GLAD_INCLUDES glad_gl_core INCLUDE_DIRECTORIES)
message(STATUS "glad include dirs: ${GLAD_INCLUDES}")