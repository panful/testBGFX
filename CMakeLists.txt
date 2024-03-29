﻿cmake_minimum_required(VERSION 3.21)
project(test_bgfx)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/target)
set(CMAKE_CXX_STANDARD 20)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 /Zc:__cplusplus")
endif()

# GLFW
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_INSTALL OFF)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF)
add_subdirectory(3rdparty/glfw)

# BGFX
option(BGFX_DEBUG_LIBS "Use debug version of bgfx" ON)
add_library(bgfxlib INTERFACE)
target_include_directories(bgfxlib INTERFACE
    3rdparty/bgfx/include
    3rdparty/bx/include
    3rdparty/bx/include/compat/msvc)
target_link_directories(bgfxlib INTERFACE 3rdparty/bgfx/.build/win64_vs2022/bin)
if(${BGFX_DEBUG_LIBS})
    target_compile_definitions(bgfxlib INTERFACE BX_CONFIG_DEBUG=1 NOMINMAX)
    target_link_libraries(bgfxlib INTERFACE bgfxDebug bimgDebug bimg_decodeDebug bxDebug)
else()
    target_link_libraries(bgfxlib INTERFACE bgfxRelease bimgRelease bimg_decodeRelease bxRelease)
endif()

add_subdirectory("source")
