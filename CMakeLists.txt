# CMakeLists.txt
# ----------------------------------------------------------------------------
cmake_minimum_required(VERSION 4.3)
# ----------------------------------------------------------------------------
project(CoreEngine)
# ----------------------------------------------------------------------------
if (DIST_BUILD)
    add_compile_definitions(DIST_BUILD)
endif ()
# ----------------------------------------------------------------------------
option(ENABLE_IMGUI "Enable ImGui" ON)
if (CMAKE_BUILD_TYPE STREQUAL "Release" OR DIST_BUILD)
    set(ENABLE_IMGUI OFF CACHE BOOL "" FORCE)
endif ()

if (ENABLE_IMGUI)
    add_compile_definitions(ENABLE_IMGUI)
endif ()
# ----------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# ----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
# ----------------------------------------------------------------------------
include(Dependencies.cmake)
# ----------------------------------------------------------------------------
add_library(glad STATIC lib/glad/src/glad.c)
target_include_directories(glad PUBLIC lib/glad/include)
# ----------------------------------------------------------------------------
add_subdirectory(Engine)
add_subdirectory(Editor)
# ----------------------------------------------------------------------------

