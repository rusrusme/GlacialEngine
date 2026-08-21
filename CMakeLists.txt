# CMakeLists.txt
# ----------------------------------------------------------------------------
cmake_minimum_required(VERSION 4.3)
# ----------------------------------------------------------------------------
project(CoreEngine CXX C)
set(CMAKE_CXX_STANDARD 20)
# ----------------------------------------------------------------------------
option(DIST_BUILD "Build for distribution" OFF)

if (DIST_BUILD OR CMAKE_BUILD_TYPE STREQUAL "Dist")
    set(DIST_BUILD ON CACHE BOOL "" FORCE)
    set(ENABLE_IMGUI OFF CACHE BOOL "" FORCE)
    set(ENABLE_SPDLOG OFF CACHE BOOL "" FORCE)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(ENABLE_IMGUI ON CACHE BOOL "" FORCE)
    set(ENABLE_SPDLOG OFF CACHE BOOL "" FORCE)
else () # Debug
    set(ENABLE_IMGUI ON CACHE BOOL "" FORCE)
    set(ENABLE_SPDLOG ON CACHE BOOL "" FORCE)
endif ()

if (DIST_BUILD)
    add_compile_definitions(DIST_BUILD)
endif ()
if (ENABLE_IMGUI)
    add_compile_definitions(ENABLE_IMGUI)
endif ()
if (ENABLE_SPDLOG)
    add_compile_definitions(ENABLE_SPDLOG)
endif ()
# ----------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# ----------------------------------------------------------------------------
include(Dependencies.cmake)
# ----------------------------------------------------------------------------
add_library(glad STATIC lib/glad/src/glad.c)
target_include_directories(glad PUBLIC lib/glad/include)

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE lib/stb_image)
# ----------------------------------------------------------------------------
add_subdirectory(Engine)
add_subdirectory(Editor)
# ----------------------------------------------------------------------------

