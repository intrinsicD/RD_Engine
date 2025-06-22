# RDE_Project/modules/common/CMakeLists.txt

add_library(Common
        src/FileIO.cpp
        src/Log.cpp
)

add_library(RDE::Common ALIAS Common)

target_include_directories(Common
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(Common
        INTERFACE
        $<$<CONFIG:Debug>:RDE_ENABLE_ASSERTS>
)

target_link_libraries(Common
        PUBLIC
        spdlog::spdlog
)