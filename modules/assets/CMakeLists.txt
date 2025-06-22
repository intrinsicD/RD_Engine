# RDE_Project/modules/assets/CMakeLists.txt

add_library(Assets
        src/ShaderLoader.cpp
        src/TextureLoader.cpp
        src/ModelLoader.cpp
        src/MaterialLoader.cpp
)

add_library(RDE::Assets ALIAS Assets)

target_include_directories(Assets
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${glm_SOURCE_DIR}
        ${stb_SOURCE_DIR}
        ${tinyobjloader_SOURCE_DIR}/tiny_obj_loader
)

target_compile_definitions(Assets
        INTERFACE
        $<$<CONFIG:Debug>:RDE_ENABLE_ASSERTS>
)

target_link_libraries(Assets
        PUBLIC
        RDE::Common
        tinyobjloader
        yaml-cpp
)