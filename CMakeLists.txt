# CMakeLists.txt
# ----------------------------------------------------------------------------
cmake_minimum_required(VERSION 4.3)
# ----------------------------------------------------------------------------
project(CoreEngine CXX C)
set(CMAKE_CXX_STANDARD 20)
# ----------------------------------------------------------------------------
option(DEBUG_DIST "Debug distribution build for developers" OFF)
option(RELEASE_DIST "Release distribution build" OFF)

if (DEBUG_DIST)
    set(DEBUG_DIST ON CACHE BOOL "" FORCE)
elseif (RELEASE_DIST)
    set(RELEASE_DIST ON CACHE BOOL "" FORCE)
else() # Non-distribution profile
    set(ENABLE_IMGUI ON CACHE BOOL "" FORCE)
endif ()

if (DEBUG_DIST)
    add_compile_definitions(DEBUG_DIST)
elseif (RELEASE_DIST)
    add_compile_definitions(RELEASE_DIST)
endif ()

if (ENABLE_IMGUI)
    add_compile_definitions(ENABLE_IMGUI)
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
if (NOT DEBUG_DIST AND NOT RELEASE_DIST)
    add_subdirectory(Editor)
endif ()
# ----------------------------------------------------------------------------

